


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


#include "ggid.h"

static unsigned short tgid = 0;


extern char isCloneChild;


extern int wmStatus;


// implements clone booting parts of platform.h


// from platform.h
char isCloneBootPossible() {
    // children cannot serve up clone boots (don't have original files)
    return !isCloneChild;
    }



static short channel = -1;


static char cloneBootRunning = false;
static char cloneBootError = false;
static char cloneBootStarted = false;
static char cloneBootCanceled = false;
static char cloneBootStartingOver = false;

static char childUserNameSet = false;
static char cloneChildUserName[ MB_USER_NAME_LENGTH  + 1 ];


static int cloneHostState = 0;

// from platform.h
int getCloneHostState() {
    if( cloneBootError ) {
        return -1;
        }
    
    return cloneHostState;
    }


// from platform.h
const char *getCloneChildUserName() {
    if( childUserNameSet ) {
        return cloneChildUserName;
        }
    else {
        return "";
        }
    }






// game info sent out to child
const MBGameRegistry mbGameInfo = {
    // specify NULL as program path name when clone booting
    NULL,
    // title
    (u16 *)L"Diamond Trust of London",
    // description
    (u16 *)L"Downloadable Two-Player Game",

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
        printOut( "Clone boot parent got wmEndCallback\n" );

        // done with multiboot

        cloneBootRunning = false;


        if( cloneBootCanceled ) {
            // do nothing, canceled

            printOut( "wmEnd after manual cancel, doing nothing\n" );
            }
        else {
            printOut( "wmEnd after successful multiboot, "
                      "accepting a connection\n" );

            // got here after a successful multiboot!

            // start accepting connections to transfer data to child
            acceptConnection();
            }
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
        // 37 traffic units used during Download Play transfer
        // DL can succeed with only 15 units, however
        // (measured with WMtest tool)
        startMeasureChannel( measureChannelCallback, 15 );
        }
    }




// from platform.h
void acceptCloneDownloadRequest() {

    // clear any errors, but DO NOT go into state 0, because we're
    // actually not accepting a connection yet (the clone boot transfer
    //  is separate)
    wmStatus = -2;

    cloneBootRunning = true;
    cloneBootError = false;
    cloneBootStarted = false;
    cloneBootCanceled = false;
    cloneBootStartingOver = false;

    cloneHostState = 1;

    childUserNameSet = false;
    

    printOut( "Getting TGID\n" );
    
    tgid = WM_GetNextTgid();
    
    printOut( "Tgid = %d\n", (int)tgid );


    WMErrCode result = WM_Initialize( &wmBuffer, wmInitializeCallback, 
                                      WM_DMA_NO );
    if( result != WM_ERRCODE_OPERATING ) {
        cloneBootError = true;
        }
    
    }


// from platform.h
void cancelCloneHosting() {
    printOut( "Got cancelCloneHosting request\n" );
    
    if( cloneBootRunning ) {
        printOut( "Clone Boot still running, trying to cancel it\n" );

        // still running MB, cancel it
        cloneBootCanceled = true;
        MBP_Cancel();
        }
    else {
        // not doing MB anymore?  Maybe accepting a connection?
        printOut( "Clone Boot already done, "
                  "trying to close connection instead\n" );
        closeConnection();
        }
    }




// from platform.h
char isCloneBootRunning() {
    return cloneBootRunning;
    }





// these two functions are NOT in platform.h and are DS-specific
// therefore, safe to stick them in PARENT region only, because we can
// control when they are called.

#include <nitro/parent_begin.h>






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
            cloneHostState = 2;



            // all completed download?
            if( MBP_IsBootableAll() ) {
                // start boot process
                MBP_StartRebootAll();
                }
            else {

                u16 downloadingBMP = 
                    MBP_GetChildBmp( MBP_BMPTYPE_DOWNLOADING );

                if( downloadingBMP != 0 && 
                    ! childUserNameSet ) {

                    // a child is downloading, an its name hasn't been
                    // retreived yet

                    u16 childAID = 1;
                    
                    // look for "1" bit position of child in BMP
                    while( ( ( downloadingBMP >> childAID ) & 1 ) 
                           == 0 ) {
                        childAID ++;
                        }
                    

                    const MBPChildInfo *childInfo = 
                        MBP_GetChildInfo( childAID );
                    
                    MBUserInfo userInfo = childInfo->user;
                    
                    for( int i=0; i<userInfo.nameLength; i++ ) {

                        // convert from unicode to ASCII
                        cloneChildUserName[i] = (char)( userInfo.name[i] );
                        }
                    cloneChildUserName[ userInfo.nameLength ] = '\0';
                    childUserNameSet = true;
                    }
                

                // there's a bug in MBP that doesn't properly handle
                // case where child drops out mid-download
                // After that happens, MBP kicks all future children
                
                // so, we need to look for these and handle them as errors

                if( downloadingBMP == 0 ) {
                    printOut( "Detected Clone Child download failure before "
                              "bootable.\n" );
                    printOut( "Canceling Clone Boot process to start over\n" );

                    // our only recourse here is to cancel the whole thing
                    // and start over
                    cloneBootStartingOver = true;
                    MBP_Cancel();
                    }
                }
            break;
        case MBP_STATE_REBOOTING:
            break;
        case MBP_STATE_COMPLETE:
            // done with success, child is booted


            cloneHostState = 3;
            
            // shut down WM so we can accept a fresh connection
            WM_End( wmEndCallback );
            break;
        case MBP_STATE_ERROR:
            printOut( "MBP error received by Clone Parent\n" );
            
            childUserNameSet = false;
            
            // cancel MB before reporting error
            MBP_Cancel();
            break;
        case MBP_STATE_CANCEL:
            // cancel in progress, do nothing
            break;
        case MBP_STATE_STOP:
            if( cloneBootStartingOver ) {
                printOut( "MBP Stopped, trying to start over\n" );
                cloneBootStartingOver = false;
                cloneHostState = 1;
                
                childUserNameSet = false;

                // use same settings again
                MBP_Init( LOCAL_GGID, tgid );
                }
            else if( !cloneBootCanceled ) {
                // only hit this on error
                cloneBootError = true;

                childUserNameSet = false;
                }
            else {
                // canceled
                // shut down WM for good
                WM_End( wmEndCallback );
                }
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



#include <nitro/parent_end.h>

