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








#ifdef SDK_TWL
    void TwlMain( void )    
#else
    void NitroMain( void )
#endif
{

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
    
    

    while( true ){
        }
    

    OS_Terminate();
    
    }
