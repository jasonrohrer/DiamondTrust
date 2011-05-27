#include "platform.h"

#define true 1

extern char isCloneChild;


// called at beginning of gameInit to check if this is a clone boot
// child and fetch clone data if so
void checkCloneFetch() {

    if( !MB_IsMultiBootChild() ) {
        return;
        }

    /*
    char keepLooping = true;
    while( keepLooping ) {
        // busy loop to allow breaking into child to set breakpoints
        }
    */


    isCloneChild = true;

    printOut( "Multiboot child started\n" );

    //printOut( "Trying to connect back to server...\n" );

    printOut( "Waiting until game loads to auto-connect to parent\n" );
    return;
    



    // connect to server so that we can get files from it
    connectToServer( NULL );

    while( checkConnectionStatus() == 0 ) {
        }
    
    if( checkConnectionStatus() == 1 ) {
        printOut( "Connected\n" );
        }
    else {
        printOut( "Error on connect\n" );
        }
    



    //OS_Panic( "Multi-boot child panic\n" );
    }
