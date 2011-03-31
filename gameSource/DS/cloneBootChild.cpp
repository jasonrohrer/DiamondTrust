#include "platform.h"


// called at beginning of gameInit to check if this is a clone boot
// child and fetch clone data if so
void checkCloneFetch() {

    if( !MB_IsMultiBootChild() ) {
        return;
        }
    OS_Panic( "Multi-boot child started\n" );
    }
