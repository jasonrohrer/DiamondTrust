#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
#endif


#define true 1



#include "platform.h"

// implement plaform functions

void *allocMem( unsigned int inSizeInBytes ) {
    return OS_Alloc( inSizeInBytes );
    }


void freeMem( void *inRegion ) {
    OS_Free( inRegion );
    }


unsigned char *readFile( char *inFileName, int *outSize ) {
    FSFile file;
    
    if( FS_OpenFileEx( &file, inFileName, FS_FILEMODE_R ) ) {
        unsigned int length = FS_GetLength( &file );
        
        unsigned char *buffer = (unsigned char *)allocMem( length );
        
        int numRead = FS_ReadFile( &file, buffer, (int)length );
        
        if( numRead != length ) {
            freeMem( buffer );
            buffer = NULL;
            *outSize = -1;
            }
        else {
            *outSize = numRead;
            }
        
        FS_CloseFile( &file );

        return buffer;
        }
    else {
        *outSize = -1;
        return NULL;
        }
    
    }




// stuff for drawing 3D on both screens (one screen each vblank)

char shouldSwap = false;
char isEvenFrame = false;

GXOamAttr oamAttr[ 128 ];

// thread that waits for a safe time to swap
#define SWAP_THREAD_STACK_SIZE  1024
#define SWAP_THREAD_PRIORITY  ( OS_THREAD_LAUNCHER_PRIORITY - 6 )

OSThread swapThread;
unsigned int swapThreadStack[ SWAP_THREAD_STACK_SIZE /
                              sizeof( unsigned int ) ];
void swapThreadProcess( void *inData );



// functions called by thread to switch drawing modes between frames


static void setupEvenFrame() {

    GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

    GX_ResetBankForSubOBJ();
    GX_SetBankForSubBG( GX_VRAM_SUB_BG_128_C );
    GX_SetBankForLCDC( GX_VRAM_LCDC_D);

    GX_SetCapture( GX_CAPTURE_SIZE_256x192,
                   GX_CAPTURE_MODE_A,
                   GX_CAPTURE_SRCA_2D3D, 
                   (GXCaptureSrcB)0, 
                   GX_CAPTURE_DEST_VRAM_D_0x00000, 16, 0 );

    GX_SetGraphicsMode( GX_DISPMODE_VRAM_D, GX_BGMODE_0, GX_BG0_AS_3D ); 
    
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
    G2_SetBG0Priority( 0 );

    GXS_SetGraphicsMode( GX_BGMODE_5 );
    GXS_SetVisiblePlane( GX_PLANEMASK_BG2 );
    
    G2S_SetBG2ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256,
                            GX_BG_AREAOVER_XLU, 
                            GX_BG_BMPSCRBASE_0x00000 );
    
    G2S_SetBG2Priority( 0 );
    G2S_BG2Mosaic( FALSE );
    }


static void setupOddFrame() {
    GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
    GX_ResetBankForSubBG();
    GX_SetBankForSubOBJ( GX_VRAM_SUB_OBJ_128_D );
    GX_SetBankForLCDC( GX_VRAM_LCDC_C );

    GX_SetCapture( GX_CAPTURE_SIZE_256x192,
                   GX_CAPTURE_MODE_A,
                   GX_CAPTURE_SRCA_2D3D,
                   (GXCaptureSrcB)0, 
                   GX_CAPTURE_DEST_VRAM_C_0x00000, 16, 0);

    
    GX_SetGraphicsMode( GX_DISPMODE_VRAM_C, GX_BGMODE_0, GX_BG0_AS_3D );

    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
    G2_SetBG0Priority( 0 );

    GXS_SetGraphicsMode( GX_BGMODE_5 );
    GXS_SetVisiblePlane( GX_PLANEMASK_OBJ );
    }



void swapThreadProcess( void *inData ) {
    // not using, compiler warns us
    void *data = inData;
    
    while( true ) {

        if( GX_GetVCount() <= 193 ) {
#ifdef SDK_TWL
            OS_SpinWaitSysCycles( 784 );
#else
            OS_SpinWait( 784 );
#endif
            }
        if( !G3X_IsGeometryBusy() && shouldSwap ) {
            if( isEvenFrame ) {
                setupEvenFrame();
                }
            else {
                setupOddFrame();
                }
            shouldSwap = false;
            isEvenFrame = !isEvenFrame;
        

            OS_SleepThread( NULL );
            }
        }
    }

    

static void VBlankCallback() {
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
    
    // wake thread up every vblank
    OS_WakeupThreadDirect( &swapThread );
    }





#ifdef SDK_TWL
    void TwlMain( void )    
#else
    void NitroMain( void )
#endif
{

    OS_Init();
    FX_Init();

    GX_Init();

    GX_DispOff();
    GXS_DispOff();



    // turn on VBLANK interrupt handling
    OS_SetIrqFunction( OS_IE_V_BLANK, VBlankCallback );
    OS_EnableIrqMask( OS_IE_V_BLANK );
    OS_EnableIrq();

    GX_VBlankIntr( true );  



    // init vram
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    MI_CpuClearFast( (void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    
    GX_DisableBankForLCDC();

    MI_CpuFillFast( (void *)HW_OAM, 192, HW_OAM_SIZE );
    MI_CpuClearFast( (void *)HW_PLTT, HW_PLTT_SIZE );

    MI_CpuFillFast( (void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE );
    MI_CpuClearFast( (void *)HW_DB_PLTT, HW_DB_PLTT_SIZE );  


    // setup our thread to do draw mode swapping
    OS_InitThread();
    OS_CreateThread( &swapThread, swapThreadProcess, NULL, 
                     swapThreadStack + 
                     SWAP_THREAD_STACK_SIZE / sizeof(unsigned int), 
                     SWAP_THREAD_STACK_SIZE,
                     SWAP_THREAD_PRIORITY );
    OS_WakeupThreadDirect( &swapThread );

    
    
    G3X_Init();
    G3X_InitTable();
    G3X_InitMtxStack();
    G3X_AntiAlias(TRUE);



    // setup OAM stuff for lower screen
    GXS_SetOBJVRamModeBmp( GX_OBJVRAMMODE_BMP_2D_W256 );

    for( int i=0; i<128; i++ ) {
        oamAttr[i].attr01 = 0;
        oamAttr[i].attr23 = 0;
        }
    
    int oamID = 0;
    
    for( int y=0; y<192; y+=64) {
        for( int x=0; x<256; x+=64 ) {
            G2_SetOBJAttr( &oamAttr[ oamID ],
                           x,
                           y,
                           0,
                           GX_OAM_MODE_BITMAPOBJ,
                           FALSE,
                           GX_OAM_EFFECT_NONE,
                           GX_OAM_SHAPE_64x64,
                           GX_OAM_COLOR_16, 
                           (y / 8) * 32 + (x / 8), 
                           15, 
                           0 );
            oamID++;
            }
        }

    DC_FlushRange( &oamAttr[0], sizeof(oamAttr) );

    GXS_LoadOAM( &oamAttr[0], 0, sizeof(oamAttr) );
    


    
    // setup 3D
    G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3X_SetClearColor( GX_RGB( 0, 0, 0 ),
                       0,
                       0x7fff,
                       63,
                       FALSE );

    G3_ViewPort( 0, 0, 255, 191 );

    // projection matrix
    G3_Perspective( FX32_SIN30, FX32_COS30,
                    FX32_ONE * 4 / 3,
                    FX32_ONE,
                    FX32_ONE * 400,
                    NULL );
    G3_StoreMtx( 0 );


    // 2d/3d blending
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0,
                      GX_BLEND_PLANEMASK_BG0 |
                      GX_BLEND_PLANEMASK_BG1 |
                      GX_BLEND_PLANEMASK_BG2 |
                      GX_BLEND_PLANEMASK_BG3 |
                      GX_BLEND_PLANEMASK_OBJ |
                      GX_BLEND_PLANEMASK_BD,
                      16,
                      0 );



    // setup memory heap    
    OS_Printf( "Setting up heap\n" );
    void *arenaLo = OS_InitAlloc( OS_ARENA_MAIN, 
                                  OS_GetMainArenaLo(),
                                  OS_GetMainArenaHi(), 1 );

    OS_SetArenaLo( OS_ARENA_MAIN, arenaLo );

    OSHeapHandle heapHandle = OS_CreateHeap( OS_ARENA_MAIN, 
                                             OS_GetMainArenaLo(), 
                                             OS_GetMainArenaHi() );
    if( heapHandle < 0 ) {
        OS_TPanic( "Failed to create main heap.\n");
        }

    OS_SetCurrentHeap( OS_ARENA_MAIN, heapHandle );
    
    OS_EnableIrq();

    OS_Printf( "Init file system\n" );
    FS_Init( 3 );



    OS_Printf( "Hello, World\n" );
    

    // test file system
    int fileSize;
    unsigned char *fileContents = readFile( "test.txt", &fileSize );
    
    if( fileContents != NULL ) {
        for( int i=0; i<fileSize; i++ ) {
            OS_Printf( "%c", fileContents[ i ] );
            }
        freeMem( fileContents );
        }
    else {
        OS_Printf( "File read failed\n" );
        }
    
    
    // start display
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();

    while( true ){
        G3X_Reset();

        // camera
        VecFx32 cameraPos = { 0, 0, FX32_ONE * 5 };
        VecFx32 lookAt = { 0, 0, 0 };
        VecFx32 upVector = { 0, FX32_ONE, 0 };

        G3_LookAt( &cameraPos, &upVector, &lookAt, NULL );
        G3_PushMtx();

        if( isEvenFrame ) {
            drawTopScreen();
            }
        else {
            drawBottomScreen();
            }
        G3_PopMtx( 1 );

        // swap buffers
        OSIntrMode oldMode = OS_DisableInterrupts();
        G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
        shouldSwap = true;
        OS_RestoreInterrupts( oldMode );
        
        // interrupt will wake swapThread up
        OS_WaitVBlankIntr();
        }
    

    OS_Terminate();
    
    }
