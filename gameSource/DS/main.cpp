#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
#endif


#define true 1


#include <string.h>

#include "platform.h"
#include "common.h"

// implement plaform functions


void* operator new ( std::size_t inSizeInBytes ) {
    return OS_Alloc( inSizeInBytes );
    }


void* operator new[] ( std::size_t inSizeInBytes ) {
    return OS_Alloc( inSizeInBytes );
    }


void operator delete ( void *inRegion ) throw() {
    OS_Free( inRegion );
    }


void operator delete[] ( void *inRegion ) throw() {
    OS_Free( inRegion );
    }


// called by system before C++ static constructor calls
// set our heap system up here for use by new and delete operators
void NitroStartUp( void ) {
    OS_Init();
    

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
    }



unsigned char *readFile( char *inFileName, int *outSize ) {
    FSFile file;
    
    if( FS_OpenFileEx( &file, inFileName, FS_FILEMODE_R ) ) {
        unsigned int length = FS_GetLength( &file );
        
        unsigned char *buffer = new unsigned char[ length ];
        
        int numRead = FS_ReadFile( &file, buffer, (int)length );
        
        if( numRead != length ) {
            delete [] buffer;
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



void printOut( const char *inFormatString, ... ) {
    va_list argList;
    va_start( argList, inFormatString );

    
    //OS_VPrintf( inFormatString, argList );
    // simpler to reduce stack overhead
    OS_TVPrintf( inFormatString, argList );
    
    va_end( argList );
    }



MATHRandContext32 randContext;

unsigned int getRandom( unsigned int inMax ) {
    return MATH_Rand32( &randContext, inMax );
    }




//#define MAX_TEXTURES  512
#define MAX_TEXTURES  1024

struct textureInfoStruct {
        unsigned int slotAddress;
        GXTexFmt texFormat;
        // if format is DIRECT, this is not set:
        unsigned int paletteSlotAddress;
        char colorZeroTransparent;
        int w;
        int h;
        GXTexSizeS sizeS;
        GXTexSizeT sizeT;
        GXSt texCoordCorners[4];
    };

typedef struct textureInfoStruct textureInfo;

textureInfo textureInfoArray[ MAX_TEXTURES ];

int nextTextureInfoIndex = 0;
unsigned int nextTextureSlotAddress = 0x0000;
unsigned int nextTexturePaletteAddress = 0x0000;

int numTextureBytesAdded = 0;
int numTexturePaletteBytesAdded = 0;

// used as temporary storage when adding a sprite to build up a palette
unsigned short textureColors[ 256 ] ATTRIBUTE_ALIGN(4);




static textureInfo makeTextureInfo( int inWidth, int inHeight ) {
    textureInfo t;
    
    t.slotAddress = nextTextureSlotAddress;
    t.w = inWidth;
    t.h = inHeight;
    
    //printOut( "Adding %dx%d texture at address %d, handle %d\n", 
    //          t.w, t.h, t.slotAddress, nextTextureInfoIndex );
    

    switch( inWidth ) {
        case 8:
            t.sizeS = GX_TEXSIZE_S8;
            break;
        case 16:
            t.sizeS = GX_TEXSIZE_S16;
            break;
        case 32:
            t.sizeS = GX_TEXSIZE_S32;
            break;
        case 64:
            t.sizeS = GX_TEXSIZE_S64;
            break;
        case 128:
            t.sizeS = GX_TEXSIZE_S128;
            break;
        case 256:
            t.sizeS = GX_TEXSIZE_S256;
            break;
        case 512:
            t.sizeS = GX_TEXSIZE_S512;
            break;
        case 1024:
            t.sizeS = GX_TEXSIZE_S1024;
            break;
        default:
            printOut( "Unsupported texture width, %d\n", inWidth );
        }
    switch( inHeight ) {
        case 8:
            t.sizeT = GX_TEXSIZE_T8;
            break;
        case 16:
            t.sizeT = GX_TEXSIZE_T16;
            break;
        case 32:
            t.sizeT = GX_TEXSIZE_T32;
            break;
        case 64:
            t.sizeT = GX_TEXSIZE_T64;
            break;
        case 128:
            t.sizeT = GX_TEXSIZE_T128;
            break;
        case 256:
            t.sizeT = GX_TEXSIZE_T256;
            break;
        case 512:
            t.sizeT = GX_TEXSIZE_T512;
            break;
        case 1024:
            t.sizeT = GX_TEXSIZE_T1024;
            break;
        default:
            printOut( "Unsupported texture height, %d\n", inHeight );
        }
    
    t.texCoordCorners[0] = GX_ST( 0, inHeight << FX32_SHIFT );
    t.texCoordCorners[1] = GX_ST( 0, 0 );
    t.texCoordCorners[2] = GX_ST( inWidth << FX32_SHIFT,  0 );
    t.texCoordCorners[3] = GX_ST( inWidth << FX32_SHIFT,  
                                  inHeight << FX32_SHIFT );

    return t;
    }



static void addPalette( textureInfo *inT, 
                        unsigned short *inPalette, 
                        unsigned int inPaletteEntries ) {
    
    inT->paletteSlotAddress = nextTexturePaletteAddress;


    // copy into aligned memory
    unsigned int paletteBytes = inPaletteEntries * sizeof( short );
    
    memcpy( (void *)textureColors, (void *)inPalette, paletteBytes );
    
    

    DC_FlushRange( textureColors, paletteBytes );
    
    GX_BeginLoadTexPltt();
    GX_LoadTexPltt( textureColors,
                    inT->paletteSlotAddress,
                    paletteBytes );
    GX_EndLoadTexPltt();

    unsigned int offset = paletteBytes;

    if( offset < 16 ) {
        // offset to next palette must be 16 bytes for 
        //   16- and 256-color palettes
        // so our 8-byte offset (for this 4-color palette) is
        // not enough.
        offset = 16;
        }

    nextTexturePaletteAddress += offset;
    
    numTexturePaletteBytesAdded += offset;
    printOut( "Added %d paletteBytes bytes so far.\n", 
              numTexturePaletteBytesAdded );
    }



static void addTextureData( textureInfo *inT,
                            unsigned short *inPackedTextureData,
                            unsigned int inNumBytes ) {
    
    DC_FlushRange( inPackedTextureData, inNumBytes );

    GX_BeginLoadTex();
    
    GX_LoadTex( inPackedTextureData, 
                inT->slotAddress,
                inNumBytes );
    
    GX_EndLoadTex(); 

    

    
    nextTextureSlotAddress += inNumBytes;
    
    numTextureBytesAdded += inNumBytes;
    }




int addSprite256( unsigned char *inDataBytes, int inWidth, int inHeight,
                  unsigned short inPalette[256], char inZeroTransparent ) {
    
    unsigned int numPixels = (unsigned int)( inWidth * inHeight );
    
    textureInfo t = makeTextureInfo( inWidth, inHeight );

    t.texFormat = GX_TEXFMT_PLTT256;
    
    addPalette( &t, inPalette, 256 );
    
    addTextureData( &t, (unsigned short*)inDataBytes, numPixels );
    
    t.colorZeroTransparent = inZeroTransparent;

    textureInfoArray[ nextTextureInfoIndex ] = t;
    int returnIndex = nextTextureInfoIndex;
    
    nextTextureInfoIndex++;
    
    return returnIndex;
    }


void replaceSprite256( int inSpriteID, 
                       unsigned char *inDataBytes, 
                       int inWidth, int inHeight ) {

    unsigned int numPixels = (unsigned int)( inWidth * inHeight );

    addTextureData( &( textureInfoArray[ inSpriteID ] ), 
                    (unsigned short*)inDataBytes, numPixels );
    }




int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight ) {
    
    textureInfo t = makeTextureInfo( inWidth, inHeight );
    
    

    unsigned int numPixels = (unsigned int)( inWidth * inHeight );
    
    unsigned short *textureData = new unsigned short[ numPixels ];

    // track palette indices for each pixel, even if we don't end up
    // using it
    unsigned char *texturePaletteIndices =  new unsigned char[ numPixels ];
    

    // zero palette (so unused colors are 0 later)
    for( int k=0; k<256; k++ ) {
        textureColors[k] = 0;
        }

    // reserve color 0 for transparency, but only if it's used!
    // check
    char transUsed = false;
    
    for( int i=0; i<numPixels && !transUsed; i++ ) {
        if( inDataRGBA[i].a == 0 ) {
            transUsed = true;
            }
        }
    
    int numUniqueColors = 0;
    
    if( transUsed ) {
        // 0 already set asside for all transparent pixels
        numUniqueColors = 1;
        }
    

    for( int i=0; i<numPixels; i++ ) {
        // discard lowest 3 bits for each color
        // discard all but 1 bit for alpha
        
        rgbaColor c = inDataRGBA[i];
        
        textureData[i] = (unsigned short)( 
            ( c.a >> 7 ) << 15 |
            ( c.b >> 3 ) << 10 |
            ( c.g >> 3 ) << 5 |
            ( c.r >> 3 ) );
        
        if( numUniqueColors < 257 ) {
            
            // unique?
            unsigned short c16 = textureData[i];

            char found = false;

            if( transUsed ) {    
                int alpha = c.a >> 7;
                if( alpha == 0 ) {
                    // matches palette color 0
                    texturePaletteIndices[i] = 0;
                    found = true;
                    }
                }
            
            
            int jLimit = numUniqueColors;
            if( jLimit > 256 ) {
                jLimit = 256;
                }
            
            int startColorIndex = 0;
            if( transUsed ) {
                startColorIndex = 1;
                }
            
            for( int j=startColorIndex; j<jLimit && !found; j++ ) {
                if( c16 == textureColors[ j ] ) {
                    found = true;
                    texturePaletteIndices[i] = (unsigned char)j;
                    }
                }
            if( !found ) {
                
                if( numUniqueColors < 256 ) {
                    // add to palette
                    textureColors[ numUniqueColors ] = c16;                    
                    texturePaletteIndices[i] = (unsigned char)numUniqueColors;
                    //printOut( "Adding color to palette: " );
                    //printColor( c );
                    //printOut( "\n" );
                    }
                // this may push us past 256, in which case
                // we cannot use a palette for this texture
                numUniqueColors ++;
                }
            }
        
        }
    

    unsigned short *packedTextureData = NULL;
    unsigned int numTextureShorts;
    

    if( numUniqueColors > 256 ) {
        // direct color
        //printOut( "Texture requires direct color\n" );

        t.texFormat = GX_TEXFMT_DIRECT;
        
        numTextureShorts = numPixels;
        packedTextureData = textureData;

        // only use color zero as transparent for paletted textures
        // (direct color textures have 1-bit alpha)
        t.colorZeroTransparent = false;        
        }
    else {
        // force zero transparent for paletted textures if trans used
        t.colorZeroTransparent = transUsed;
        
        //printOut( "Texture uses only %d colors\n", numUniqueColors );
        
        // palette colors are RGB 555 (alpha bit (bit 15) ignored... set to
        // 1 just to be safe
        for( int k=0; k<numUniqueColors; k++ ) {
            textureColors[k] |= 0x8000;
            }

        // don't need direct colors anymore
        delete [] textureData;

        unsigned int paletteSize = 0;
        
        if( numUniqueColors <= 4 ) {
            t.texFormat = GX_TEXFMT_PLTT4;
            paletteSize = 4;
            
            // pack into 2 bits per pixel (8 pixels per short)
            numTextureShorts = numPixels / 8;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<8; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 2)) );
                    }
                }
            }
        else if( numUniqueColors <= 16 ) {
            t.texFormat = GX_TEXFMT_PLTT16;
            paletteSize = 16;
            
            // pack into 4 bits per pixel (4 pixels per short)
            numTextureShorts = numPixels / 4;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<4; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 4)) );
                    }
                }
            }
        else if( numUniqueColors <= 256 ) {
            t.texFormat = GX_TEXFMT_PLTT256;
            paletteSize = 256;
            
            // pack into 8 bits per pixel (2 pixel per short)
            numTextureShorts = numPixels / 2;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<2; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 8)) );
                    }
                }
            }
        
        //printOut( "Using a %d color palette\n", paletteSize );


        

        t.paletteSlotAddress = nextTexturePaletteAddress;

        addPalette( &t, textureColors, paletteSize );
        }
    

    unsigned int numTextureBytes = numTextureShorts * 2;
    addTextureData( &t, packedTextureData, numTextureBytes );

    
    textureInfoArray[ nextTextureInfoIndex ] = t;
    int returnIndex = nextTextureInfoIndex;
    
    nextTextureInfoIndex++;
    
    
    printOut( "Added %d texture bytes so far (last texture had %d colors).\n",
              numTextureBytesAdded, numUniqueColors );
    
    delete [] packedTextureData;
    
    delete [] texturePaletteIndices;
    
    return returnIndex;
    }



// fake z buffer values to force proper sorting (later sprites layers on top)
fx16 farZ = -7 * FX16_ONE;
fx16 drawZ = farZ;
fx16 drawZIncrement = 0x0004;

int polyID = 0;


void startNewSpriteLayer() {
    drawZ += drawZIncrement;

    // also start a new ID group 
    polyID++;
    if( polyID > 63 ) {
        polyID = 0;
        }
    }



void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor ) {
    //if( true ) return;
    
    textureInfo t = textureInfoArray[ inHandle ];
    
    //printOut( "Drawing sprite at address %d\n", t.slotAddress );


    GXTexPlttColor0 colorZeroFlag;
    if( ! t.colorZeroTransparent ) {
        colorZeroFlag = GX_TEXPLTTCOLOR0_USE;
        }
    else {
        colorZeroFlag = GX_TEXPLTTCOLOR0_TRNS;
        }
    
    G3_TexImageParam( t.texFormat,
                      GX_TEXGEN_TEXCOORD,
                      t.sizeS,
                      t.sizeT,
                      GX_TEXREPEAT_NONE,
                      GX_TEXFLIP_NONE,
                      colorZeroFlag,
                      t.slotAddress );
    
    
    if( t.texFormat != GX_TEXFMT_DIRECT ) {        
        G3_TexPlttBase( t.paletteSlotAddress,
                        t.texFormat );
        }
    
    
    // 5 high-order bits
    int a = inColor.a >> 3;
    
    
    // avoid wireframe
    if( a == 0 ) {
        // draw nothing
        return;
        }


    G3_PolygonAttr( GX_LIGHTMASK_NONE,
                    GX_POLYGONMODE_MODULATE,
                    GX_CULL_NONE,
                    polyID,
                    a,
                    0 );

    G3_PushMtx();


    G3_Translate( 0, 
                  0, 
                  drawZ );



    G3_Begin( GX_BEGIN_QUADS );

    // set color once
    G3_Color( GX_RGB( inColor.r >> 3, inColor.g >> 3, inColor.b >> 3 ) );


    // four corners

    // 16 bit fixed point not enough to hold integer values in range 0..255
    // (screen pixel coordinates) so some shifting is necessary
    // Make up for this in the setup of Ortho in main function
    // (shifting by 6 is like dividing by 64, and 256x192 divided by 64
    //   is 4x3, which is used in Ortho).

    // Note that this was the ONLY coordinate conversion method that didn't
    // result in distortion as polygons moved in y direction.

    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[0] );
    G3_Vtx( (short)( inX << (FX16_SHIFT - 6) ), 
            (short)( (inY + t.h) << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[1] );
    G3_Vtx( (short)( inX << (FX16_SHIFT - 6) ), 
            (short)( inY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[2] );
    G3_Vtx( (short)( (inX + t.w) << (FX16_SHIFT - 6) ), 
            (short)( inY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[3] );
    G3_Vtx( (short)( (inX + t.w) << (FX16_SHIFT - 6) ), 
            (short)( (inY + t.h) << (FX16_SHIFT - 6) ), 
            0 );

    G3_End();

    G3_PopMtx( 1 );
    }



char getTouch( int *outX, int *outY ) {
    TPData result;
    
    if( TP_RequestCalibratedSampling( &result ) == 0 ) {
        if( result.touch == TP_TOUCH_ON && 
            result.validity == TP_VALIDITY_VALID ) { 
            
            *outX = result.x;
            *outY = result.y;
            return true;
            }
        }
    return false;
    }





// wireless stuff
static unsigned char wmBuffer[ WM_SYSTEM_BUF_SIZE ] ATTRIBUTE_ALIGN( 32 );

int wmStatus = 0;
char mpRunning = false;

// true for parent, false for child
char isParent = false;

#define WM_DMA_NO  2
#define LOCAL_GGID 0x003fff63

unsigned short tgid = 0;

unsigned short allowedChannels = 0;
unsigned short nextChannel = 0;

// FIXME:  replace with actual measurement of traffic when game runs
int requiredTraffic = 20;
int bestChannel = -1;
int bestBusyRatio = 101;


static WMParentParam parentParam ATTRIBUTE_ALIGN(32);

static unsigned short remoteAID = 0;

static unsigned short portNumber = 13;



// for child scanning
static WMScanParam scanParam;
static WMBssDesc scanBssDesc;

// FIXME:  add later to send parent specific data on connect
// NULL for now (all zeros sent)
//static unsigned char childSSID[ WM_SIZE_CHILD_SSID ];



static unsigned char *sendBuffer = NULL;
static unsigned char *receiveBuffer = NULL;



static void wmEndCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndCallback\n" );
        }
    else {
        if( sendBuffer != NULL ) {
            delete [] sendBuffer;
            sendBuffer = NULL;
            }
        if( receiveBuffer != NULL ) {
            delete [] receiveBuffer;
            receiveBuffer = NULL;
            }
        }
    }



static void wmDisconnectCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmDisconnectCallback\n" );
        }
    else {
        WM_End( wmEndCallback );
        }
    }


static void disconnect() {
    if( isParent ) {
        WM_Disconnect( wmDisconnectCallback, remoteAID );
        }
    else {
        // parent AID is 0
        WM_Disconnect( wmDisconnectCallback, 0 );
        }
    }



static void wmEndMPCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndMPCallback\n" );
        }
    else {
        mpRunning = false;
        //disconnect();
        }
    }



#include "DataFifo.h"


DataFifo sendFifo;
DataFifo receiveFifo;

char sendPending = false;





static void wmPortCallback( void *inArg ) {
    WMPortRecvCallback *callbackArg = (WMPortRecvCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error returned to wmPortCallback\n" );

        wmStatus = -1;
        WM_EndMP( wmEndMPCallback );
        }
    else {
        if( callbackArg->state == WM_STATECODE_PORT_RECV ) {
            printOut( "wmPortCallback getting message of length %d\n",
                      callbackArg->length );
            
            receiveFifo.addData( (unsigned char*)callbackArg->data, 
                                 (unsigned int)callbackArg->length );
            }
        }
    }



void startNextSend();

static void dummyC() {
    startNextSend();
    }

static void dummyB() {
    dummyC();
    }

static void dummyA() {
    dummyB();
    }


static void wmStartMPCallback( void *inArg ) {
    WMStartMPCallback *callbackArg = (WMStartMPCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {

        if( callbackArg->errcode == WM_ERRCODE_SEND_FAILED ) {
            printOut( "Send failed during MP, trying to continue\n" );
            }
        else if( callbackArg->errcode == WM_ERRCODE_TIMEOUT ) {
            printOut( "Timeout during MP, trying to continue\n" );
            }
        else {    
            wmStatus = -1;
            printOut( "Fatal error %d returned to wmStartMPCallback\n",
                      callbackArg->errcode );
            //disconnect();
            }
        }
    else {
        if( callbackArg->state == WM_STATECODE_MP_START ) {
            printOut( "MP Started\n" );
            
            wmStatus = 1;
            mpRunning = true;
            
            
            // prepare to receive data
            WMErrCode result = WM_SetPortCallback(
                portNumber, wmPortCallback, NULL );

            if( result !=  WM_ERRCODE_SUCCESS ) {
                wmStatus = -1;
                WM_End( wmEndCallback );
                }
            else {
                // send any data that is waiting to go
                //startNextSend();
                //dummyA();
                }
            
            }
        }
    }


static void startMP() {
    
    if( sendBuffer != NULL ) {
        delete [] sendBuffer;
        sendBuffer = NULL;
        }
    if( receiveBuffer != NULL ) {
        delete [] receiveBuffer;
        receiveBuffer = NULL;
        }


    unsigned short sendBufferSize = 
        (unsigned short)WM_GetMPSendBufferSize();
    unsigned short receiveBufferSize = 
        (unsigned short)WM_GetMPReceiveBufferSize();
    printOut( "Send,receive buffer sizes are: %d,%d\n", 
              sendBufferSize,
              receiveBufferSize );
                 
    // make sure they are sane before allocating memory based
    // on them, because these sizes come from what the
    // parent sends
    if( sendBufferSize > 1920 || receiveBufferSize > 1920 ) {
        // bad sizes, bail out    
        wmStatus = -1;
        WM_End( wmEndCallback );
        return;
        }
                
    // sizes okay to allocate memory with
    // allocations are automatically 32-byte aligned
    sendBuffer = 
        new unsigned char[ sendBufferSize ];
    receiveBuffer = 
        new unsigned char[ receiveBufferSize ];

    // start multi port communications
    WMErrCode result =
        WM_StartMP(
            wmStartMPCallback,
            (u16*)receiveBuffer,
            receiveBufferSize,
            (u16*)sendBuffer,
            sendBufferSize,
            1 );  // one communication per frame
                
    if( result !=  WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        }

    }




static void wmStartParentCallback( void *inArg ) {
    WMStartParentCallback *callbackArg = (WMStartParentCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmStartParentCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        switch( callbackArg->state ) {
            case WM_STATECODE_PARENT_START: {
                
                
                }
                break;
            case WM_STATECODE_CONNECTED: {
                
                
                // don't start multiplayer until we're connected
                remoteAID = callbackArg->aid;
                printOut( "Parent sees child remote AID of %d\n", remoteAID );
                
                startMP();

                // FIXME:
                //parent_load_status();
                
                }
                return;
            case WM_STATECODE_DISCONNECTED:

                printOut( "Disconnected in wmStartParent\n" );
                
                // FIXME:
                //parent_load_status();
                    
                if( mpRunning ) {
                    printOut( "Waiting for child to connect again\n" );
                    mpRunning = false;
                    WM_EndMP( wmEndMPCallback );
                    }
                
                return;
                    
            default:
                    
                return;
            }
        }
    }



    


static void wmSetParentParamCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmSetParentParamCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        WMErrCode result = WM_StartParent( wmStartParentCallback );
        
        if( result != WM_ERRCODE_OPERATING ) {
            wmStatus = -1;
            WM_End( wmEndCallback );
            }
        }
    
    }


void measureNextChannel();



static void wmMeasureChannelCallback( void *inArg ) {
    WMMeasureChannelCallback *callbackArg = ( WMMeasureChannelCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmMeasureChannelCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // ignore channels too busy for our traffic
        if( callbackArg->ccaBusyRatio + requiredTraffic < 100 ) {
            if( callbackArg->ccaBusyRatio < bestBusyRatio ) {
                // best yet
                bestBusyRatio = callbackArg->ccaBusyRatio;
                bestChannel = callbackArg->channel;
                }
            }
        if( nextChannel < 12 ) {
            nextChannel ++;
            measureNextChannel();
            }
        else {
            // done measuring

            if( bestChannel == -1 ) {
                wmStatus = -1;
                WM_End( wmEndCallback );

                return;
                }
            else {
                // found one
                printOut( "Picking best channel:  %d\n", bestChannel );
                                

                // set parent params
                // FIXME:  use userGameInfo to specify identifying
                // info for child lobby, like parent name
                parentParam.userGameInfo = NULL;
                parentParam.userGameInfoLength = 0;
                parentParam.tgid = tgid;
                parentParam.ggid = LOCAL_GGID;
                parentParam.channel = (unsigned short)bestChannel;
                parentParam.beaconPeriod = WM_GetDispersionBeaconPeriod();
                parentParam.parentMaxSize = 512;
                parentParam.childMaxSize = 512;
                parentParam.maxEntry = 1;
                parentParam.CS_Flag = 0;
                parentParam.multiBootFlag = 0;
                parentParam.entryFlag = 1;
                parentParam.KS_Flag = 0;
            
                WMErrCode result =
                    WM_SetParentParameter( wmSetParentParamCallback,  
                                           &parentParam );
                if( result != WM_ERRCODE_OPERATING ) {
                    wmStatus = -1;
                    WM_End( wmEndCallback );
                    }
                }
            }
        
        }
    }



void measureNextChannel() {
    if( allowedChannels == 0 ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        return;
        }
    while( ! ( (1 << nextChannel) & allowedChannels ) ) {
        nextChannel++;
        
        // 14 broken?
        if( nextChannel >= 13 ) {
            return;
            }
        }
    
            
    
    WMErrCode result = 
        WM_MeasureChannel(
            wmMeasureChannelCallback,
            3,
            17,
            (unsigned short)( nextChannel + 1 ),
            30 );
    
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        }
    }





static void wmStartConnectCallback( void *inArg );


// connect to the game that we found
static void startConnect() {
    WMErrCode result = WM_StartConnect(
        wmStartConnectCallback,
        &scanBssDesc,
        // FIXME:
        // can specify a 24-byte SSID here that parent
        // can check... parent not checking, so NULL for now
        // which forces all zeros as SSID
        NULL );
    
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        printOut( "Error on startConnect\n" );
        WM_End( wmEndCallback );
        }
    }



static void wmResetCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmResetCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // reset only called from child

        // start connection process again to same parent
        printOut( "Trying to connect to parent again after reset\n" );
        startConnect();
        }
    
    }



static void wmStartConnectCallback( void *inArg ) {
    WMStartConnectCallback *callbackArg = (WMStartConnectCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        // no longer a fatal error
        //wmStatus = -1;
        printOut( "Error returned to wmStartConnectCallback\n" );

        printOut( "Trying to reset\n" );
        WM_Reset( wmResetCallback );
        }
    else {
        switch( callbackArg->state ) {
            case WM_STATECODE_CONNECTED: {
                
                startMP();
                
                //aid = cb->aid;
                }
        
                break;

            case WM_STATECODE_BEACON_LOST:
                printOut( "Beacon lost in wmStartConnect\n" );

                //__callback(CHILD_BEACONLOST, 0);
                
                break;
                
            case WM_STATECODE_DISCONNECTED:
                
                printOut( "Disconnected in wmStartConnect\n" );
                
                printOut( "Trying to connect again\n" );
                
                printOut( "Resetting connection\n" );
                
                // flag MP as not running here
                mpRunning = false;
                
                WM_Reset( wmResetCallback );
                
                //__callback(CHILD_DISCONNECTED, 0);
                
                break;
                
            default:
                break;
            }
        }
    }



static void wmEndScanCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndScanCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // ending scan only when we found a connection

        // use this game
                
        startConnect();
        

        }
    }



void scanNextChannel();


static void wmStartScanCallback( void *inArg ) {
    WMStartScanCallback *callbackArg = (WMStartScanCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmStartScanCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        if( callbackArg->state == WM_STATECODE_PARENT_FOUND ) {
            if( callbackArg->gameInfo.ggid == LOCAL_GGID ) {
 
                // use the first parent that we find (ignore rest)
                
                // end our scan before connecting
                WMErrCode result = WM_EndScan( wmEndScanCallback );
                                
                if( result != WM_ERRCODE_OPERATING ) {
                    wmStatus = -1;
                    WM_End( wmEndCallback );
                    }
                
                // Actually, if end goal is Download play only (cloneboot)
                // then these may not be relevant (one known parent, no lobby)

                // FIXME:  in future, should present a list of parents to
                // user and let them pick (lobby interface)

                // FIXME:  don't forget to use callbackArg->linkLevel
                // to display official wifi level icon in lobby interface
                return;
                }
            }

        // parent not found or ggid didn't match

        nextChannel++;
        
        if( nextChannel >= 13 ) {
            nextChannel = 0;
            }
        scanNextChannel();
        }
    }

        






void scanNextChannel() {
    if( allowedChannels == 0 ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        return;
        }
    while( ! ( (1 << nextChannel) & allowedChannels ) ) {
        nextChannel++;
        
        // 14 broken?
        // wrap back around
        if( nextChannel >= 13 ) {
            nextChannel = 0;
            }
        }
    

    scanParam.channel = (unsigned short)( nextChannel + 1 );

    DC_InvalidateRange( &scanBssDesc, sizeof(WMBssDesc) );
    
    WMErrCode result = 
        WM_StartScan( wmStartScanCallback, 
                      &scanParam );
    
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        }
    }




static void wmPortSendCallback( void *inArg ) {
    WMPortSendCallback *callbackArg = (WMPortSendCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmPortSendCallback\n" );
        WM_EndMP( wmEndMPCallback );
        }
    else {

        if( callbackArg->sentBitmap == callbackArg->destBitmap ) {
            
            printOut( "Data send successful\n" );
            delete [] callbackArg->data;
            sendPending = false;
            
            // next send will start during next network step
            }
        else {
            printOut( "Data send did not reach all recipients\n" );

            // put back on send fifo in next position to try again
            sendFifo.pushData( (unsigned char *)( callbackArg->data ), 
                               callbackArg->length );
            sendPending = false;
            
            delete [] callbackArg->data;

            if( !mpRunning ) {
                printOut( "Will try resending when MP starts again\n" );
                }
            else {
                printOut( "MP still running, trying to send again\n" );
                // next send during next network step
                }                
            }
        
        }
    }



void startNextSend() {
    
    unsigned int numBytes;
    
    unsigned char *data = sendFifo.getData( &numBytes );
    
    if( data != NULL ) {
        sendPending = true;
        
        printOut( "Starting data send of %d bytes\n", numBytes );
        
        unsigned short receiverBitmap = (unsigned short)( 1 << remoteAID );
        
        
        WMErrCode result = WM_SetMPDataToPort(
            wmPortSendCallback,
            (unsigned short *)data,
            (unsigned short)numBytes ,
            receiverBitmap,
            portNumber,
            0 );

        if( result == WM_ERRCODE_ILLEGAL_STATE ) {
            printOut( "Trying to send from illegal state\n" );
            
            printOut( "Pushing data back onto send fifo until later\n" );
            
            sendFifo.pushData( data, numBytes );

            sendPending = false;
            
            delete [] data;
            
            // what state are we in?
            WMStatus status;
            WM_ReadStatus( &status );
            
            if( status.state == WM_STATE_PARENT ||
                status.state == WM_STATE_CHILD ) {
                
                printOut( "During send:  "
                          "no longer in MP state\n" );
                
                printOut( "Will retry send after MP restarts\n" );
                
                mpRunning = false;
                
                // don't restart MP here... assume it will be restarted
                // elsewhere upon reconnect
                // startMP();
                }
            
                
            }
        
        }
    else{
        sendPending = false;
        }
    }




static void wmInitializeCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error in wmInitialize\n" );
        wmStatus = -1;
        }
    else {
        
        allowedChannels = WM_GetAllowedChannel();
        nextChannel = 0;

        if( isParent ) {
            // can't call this from inside a callback
            //tgid = WM_GetNextTgid();

            measureNextChannel();
            }
        else {
            // child
            
            // scan for parent's beacon
            
            // Scan for any MAC address
            scanParam.bssid[0] = 0xFF;
            scanParam.bssid[1] = 0xFF;
            scanParam.bssid[2] = 0xFF;
            scanParam.bssid[3] = 0xFF;
            scanParam.bssid[4] = 0xFF;
            scanParam.bssid[5] = 0xFF;
            
            scanParam.maxChannelTime = WM_GetDispersionScanPeriod();
            scanParam.scanBuf = &scanBssDesc;
            

            nextChannel = 0;
            scanNextChannel();
            
            }        
        }
    }



static void initWM() {
    
    WMErrCode result = WM_Initialize( &wmBuffer, wmInitializeCallback, 
                                      WM_DMA_NO );
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        }
    }


char isAutoconnecting() {
    return false;
    }


char *getLocalAddress() {
    return NULL;
    }


void acceptConnection() {
    isParent = true;

    printOut( "Getting TGID\n" );
    
    tgid = WM_GetNextTgid();
    
    printOut( "Tgid = %d\n", (int)tgid );

    
    initWM();
    }


void connectToServer( char *inAddress ) {
    // ignore address
    // prevent compiler warnings for unused
    inAddress = NULL;


    isParent = false;
    initWM();
    }


int checkConnectionStatus() {
    return wmStatus;
    }


void sendMessage( unsigned char *inMessage, unsigned int inLength ) {
    printOut( "Putting message of %d bytes onto send fifo\n", inLength );
    
    // add first 4 bytes, which is message length
    // must do this because DS data sharing doesn't seem to preserve
    // data length (sometimes longer than message that was sent, with
    // garbage bytes at end).
    unsigned int sendSize = inLength + 4;
    unsigned char *sendData = new unsigned char[ sendSize ];
                
    sendData[0] = (unsigned char)( ( inLength >> 24 ) & 0xFF );
    sendData[1] = (unsigned char)( ( inLength >> 16 ) & 0xFF );
    sendData[2] = (unsigned char)( ( inLength >> 8 ) & 0xFF );
    sendData[3] = (unsigned char)( ( inLength ) & 0xFF );
    
    memcpy( &( sendData[4] ), inMessage, inLength );


    sendFifo.addData( sendData, sendSize );
    delete [] sendData;
    
    // send will start during next network step
    }


unsigned char *getMessage( unsigned int *outLength ) {
    unsigned int dataLength;
    unsigned char *data = receiveFifo.getData( &dataLength );
    
    if( data == NULL ) {
        return NULL;
        }
    

    // else decode the length header at start of message

    if( dataLength < 4 ) {
        printOut( "getMessage Error:  message too short for length header\n" );
        delete [] data;
        return NULL;
        }
    
    unsigned int messageLength =
        (unsigned int)( data[0] << 24 ) |
        (unsigned int)( data[1] << 16 ) |
        (unsigned int)( data[2] << 8 ) |
        (unsigned int)( data[3] );

    // make sure size is sane
    if( messageLength > 10000000 ) {
        printOut( "Huge message size of %u received\n",
                  messageLength );
        delete [] data;
        return NULL;
        } 

    if( dataLength > messageLength + 4 ) {
        printOut( "Extra bytes padded onto end of received message:%d\n",
                  dataLength - ( messageLength + 4 ) );
        }
    
    printOut( "Message of length %d waiting in receive FIFO for getMessage\n", 
              messageLength );
    
    unsigned char *message = new unsigned char[ messageLength ];
    memcpy( message, &( data[4] ), messageLength );
    
    delete [] data;
    
    *outLength = messageLength;
    
    return message;
    }



static void stepNetwork() {
    if( !sendPending && mpRunning ) {
        // try sending next message
        startNextSend();
        }
    }




//FIXME:  camera stuff


char isCameraSupported() {
    return true;
    }


// start producing frames
void startCamera() {
    }


// stop producing frames
void stopCamera() {
    }


// Trimming size fixed at 160x120
// format fixed at grayscale 256 levels

// get the next frame
// inBuffer is where 160x120 grayscale pixels will be returned
void getFrame( unsigned char *inBuffer ) {
    }


// snap the next frame as a finished picture
void snapPicture( unsigned char *inBuffer ) {
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
            }
        
        OS_SleepThread( NULL );    
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
    
    
    // already called in NitroStartUp
    //OS_Init();
    
    OS_Printf( "Main starting\n" );


    FX_Init();
    TP_Init();
    

    MATH_InitRand32( &randContext, 13728749 );
    
    
    printOut( "Calibrating touch panel\n" );
    
    TPCalibrateParam tpCalibrate;

    if( !TP_GetUserInfo( &tpCalibrate ) ) {
        OS_Panic( "Failed to read touch panel calibration\n" );
        }

    TP_SetCalibrateParam( &tpCalibrate );
    


    GX_Init();

    GX_DispOff();
    GXS_DispOff();



    // turn on VBLANK interrupt handling
    OS_SetIrqFunction( OS_IE_V_BLANK, VBlankCallback );
    OS_EnableIrqMask( OS_IE_V_BLANK );

    GX_VBlankIntr( true );  

    OS_EnableIrq();
    OS_EnableInterrupts();


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

    // GX_SetBankForTex( GX_VRAM_TEX_0_A );
    // 256K for texture data
    GX_SetBankForTex( GX_VRAM_TEX_01_AB );

    // 64K for texture palettes
    GX_SetBankForTexPltt( GX_VRAM_TEXPLTT_0123_E ); 


    G3X_AntiAlias( TRUE );
    G3X_AlphaBlend( TRUE );
    


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
    //G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3_SwapBuffers( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    
    // Forcing A_b to max prevents polygon's A values from affecting 
    // the pixel buffer's A values, eliminating weird effects when
    // translucent polygons overlap
    G3X_SetClearColor( GX_RGB( 0, 0, 0 ),
                       31,  // A_b remains opaque no matter what is drawn
                       0x7fff,
                       63,
                       FALSE );

    G3_ViewPort( 0, 0, 255, 191 );

    G3_Ortho(
        0, 3 * FX32_ONE,
        0, 4 * FX32_ONE,
        0,
        8 * FX32_ONE,
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



    

    OS_Printf( "Init file system\n" );
    FS_Init( 3 );



    // test file system
    int fileSize;
    unsigned char *fileContents = readFile( "test.txt", &fileSize );
    
    if( fileContents != NULL ) {
        for( int i=0; i<fileSize; i++ ) {
            OS_Printf( "%c", fileContents[ i ] );
            }
        delete [] fileContents;
        }
    else {
        OS_Printf( "File read failed\n" );
        }



    
    
    // start display
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();
    
    // init in platform-independent code
    gameInit();
    
    printOut( "Free bytes on heap after init=%d\n",
              OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );
    
    while( true ){
        G3X_Reset();

        // camera
        VecFx32 cameraPos = { 0, 0, 0 };
        VecFx32 lookAt = { 0, 0, -FX32_ONE };
        VecFx32 upVector = { 0, FX32_ONE, 0 };

        G3_LookAt( &cameraPos, &upVector, &lookAt, NULL );

        G3_PushMtx();

        
        G3_MtxMode( GX_MTXMODE_TEXTURE );
        G3_Identity();
        G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

        
        // reset fake z buffer coordinate
        drawZ = farZ;
        polyID = 0;
        

        if( isEvenFrame ) {
            drawTopScreen();
            //printOut( "Free bytes on heap after drawTop=%d\n",
            //  OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );
            
            // game loop every-other screen
            gameLoopTick();
            // same for network step
            stepNetwork();
            }
        else {
            drawBottomScreen();

            //printOut( "Free bytes on heap after drawBottom=%d\n",
            //         OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );
            }
        G3_PopMtx( 1 );

        // swap buffers
        OSIntrMode oldMode = OS_DisableInterrupts();
        G3_SwapBuffers( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
        shouldSwap = true;
        OS_RestoreInterrupts( oldMode );
        
        // interrupt will wake swapThread up
        OS_WaitVBlankIntr();
        }
    

    OS_Terminate();
    
    }
