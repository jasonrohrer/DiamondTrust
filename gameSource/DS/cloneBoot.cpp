


#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
    #include <nitro/wm.h>
    #include <nitro/mb.h>
#endif


#define true 1


#include <string.h>

#include "platform.h"
#include "measureChannel.h"
#include "cloneBoot.h"


// use Nintendo's supplied mbp library for now
#include "mbp.h"

#define WM_DMA_NO  2

extern unsigned char wmBuffer[ WM_SYSTEM_BUF_SIZE ];


#define LOCAL_GGID 0x003fff63

static unsigned short tgid = 0;


// implements clone booting parts of platform.h


char isCloneBootPossible() {
    return true;
    }



static short channel = -1;


static char cloneBootRunning = false;
static char cloneBootError = false;
static char cloneBootStarted = false;




// FOR NOW:
// just send child the entire codebase
#include <nitro/parent_begin.h>

static char parentDummy = false;

#include <nitro/parent_end.h>


// game info sent out to child
const MBGameRegistry mbGameInfo = {
    // specify NULL as program path name when clone booting
    NULL,
    // title
    (u16 *)L"DiamondTrust",
    // description
    (u16 *)L"Diamond Trust of London",
    
    // FIXME!!!

    // icon character data
    "/icon.nbfc",
    // icond palette
    "/icon.nbfp",
    LOCAL_GGID,
    // 2 players (including parent
    2 };




static void wmEndCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        cloneBootError = true;
        printOut( "Error returned to wmEndCallback\n" );
        }
    else {
        // done with multiboot

        cloneBootRunning = false;

        // start accepting connections to transfer data to child
        acceptConnection();
        }
    }




static void measureChannelCallback( short inBestChannel ) {
    // done measuring

    channel = inBestChannel;
    
    // shut down WM so we can start multiboot process
    // WM_End( wmEndCallback );

    // Actually, DON'T end WM... MBP is expecting WM to be idle
    
    // ready to start multiboot
    cloneBootStarted = true;
    MBP_Init( LOCAL_GGID, tgid );
    }



static void wmInitializeCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error in wmInitialize\n" );
        cloneBootError = true;
        }
    else {
        startMeasureChannel( measureChannelCallback );
        }
    }




// from platform.h
void acceptCloneDownloadRequest() {

    cloneBootRunning = true;
    cloneBootError = false;
    cloneBootStarted = false;
    
    printOut( "Getting TGID\n" );
    
    tgid = WM_GetNextTgid();
    
    printOut( "Tgid = %d\n", (int)tgid );


    WMErrCode result = WM_Initialize( &wmBuffer, wmInitializeCallback, 
                                      WM_DMA_NO );
    if( result != WM_ERRCODE_OPERATING ) {
        cloneBootError = true;
        }
    
    }




int stepCloneBootParent() {
    
    if( cloneBootError ) {
        return -1;
        }
    if( ! cloneBootRunning ) {
        return 0;
        }
    
    if( !cloneBootStarted ) {
        // still looking for a channel?
        return 1;
        }
    
    // started, process MB state

    switch( MBP_GetState() ) {
        case MBP_STATE_IDLE:
            MBP_Start( &mbGameInfo, (unsigned short)channel );
            break;    
        case MBP_STATE_ENTRY:
            // child entry
            if( MBP_GetChildBmp( MBP_BMPTYPE_ENTRY ) ||
                MBP_GetChildBmp( MBP_BMPTYPE_DOWNLOADING ) ||
                MBP_GetChildBmp( MBP_BMPTYPE_BOOTABLE ) ) {
                
                // start child download
                MBP_StartDownloadAll();
                }
            break;
        case MBP_STATE_DATASENDING:
            // FIXME:  show status

            // all completed download?
            if( MBP_IsBootableAll() ) {
                // start boot process
                MBP_StartRebootAll();
                }
            break;
        case MBP_STATE_REBOOTING:
            // FIXME:  show status  
            break;
        case MBP_STATE_COMPLETE:
            // done with success, child is booted

            // shut down WM so we can accept a fresh connection
            WM_End( wmEndCallback );
            break;
        case MBP_STATE_ERROR:
            // cancel MB before reporting error
            MBP_Cancel();
            break;
        case MBP_STATE_CANCEL:
            // cancel in progress, do nothing
            break;
        case MBP_STATE_STOP:
            // only hit this on error
            cloneBootError = true;
            break;
        }
    
    
    return 1;
    }




void checkForFileRequest() {
    
    unsigned int messageLength;
    
    unsigned char *message = getMessage( &messageLength, 1 );
    if( message != NULL ) {
        // special case:  a file request
        
        // intercept it without placing it in received fifo


        // serve file in response
                
                
        char *fileName = new char[ messageLength + 1 ];
                    
        memcpy( fileName, message, messageLength );
                    
        fileName[ messageLength ] = '\0';
        
        delete [] message;
        
        printOut( "Parent received request for file '%s',",
                  fileName );
        
        int fileSize;
        
        unsigned char *fileData = readFile( fileName, &fileSize );
        

        
        if( fileData != NULL ) {

            // split into messages
            unsigned int messageSize = 500;
            
            unsigned int numFullMessages = fileSize / messageSize;
            unsigned int lastMessageSize = fileSize % messageSize;
            
            unsigned int numTotalMessages = numFullMessages;

            if( lastMessageSize > 0 ) {
                numTotalMessages ++;
                }

            printOut( " serving file of size %d (in %d %d-byte chunks)\n", 
                      fileSize, numTotalMessages, messageSize );
            
            // simply send the number of chunks forthcoming as the 
            // first message
            unsigned char headerMessage[4];

            headerMessage[0] = 
                (unsigned char)( ( numTotalMessages >> 24 ) & 0xFF );
            headerMessage[1] = 
                (unsigned char)( ( numTotalMessages >> 16 ) & 0xFF );
            headerMessage[2] = 
                (unsigned char)( ( numTotalMessages >> 8 ) & 0xFF );
            headerMessage[3] = 
                (unsigned char)( ( numTotalMessages ) & 0xFF );

            sendMessage( headerMessage, 4, 1 );
            
            // full chunks
            for( int i=0; i<numFullMessages; i++ ) {
                sendMessage( &( fileData[ i * messageSize ] ),
                             messageSize, 1 );
                }
            
            // final chunk
            if( lastMessageSize > 0 ) {
                sendMessage( &( fileData[ numFullMessages * messageSize ] ),
                             lastMessageSize, 1 );
                }

            delete [] fileData;
            }
        else {
            printOut( " not found\n" );
            }
        
        delete [] fileName;
        }
    }

