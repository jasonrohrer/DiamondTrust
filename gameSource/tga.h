#include "platform.h"


rgbaColor *extractTGAData( unsigned char *inData, int inNumBytes,
                           int *outWidth, int *outHeight );


rgbaColor *readTGAFile( char *inFileName, int *outWidth, int *outHeight );
