#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
#endif


#define true 1


#include <string.h>

#include "platform.h"


// implements clone booting parts of platform.h


char isCloneBootPossible() {
    return true;
    }



// starts a parent server that waits for a connection and then serve
// a clone boot executable over that connection
void acceptCloneDownloadRequest() {

    }



// called at beginning of gameInit to check if this is a clone boot
// child and fetch clone data if so
void checkCloneFetch() {

    }
