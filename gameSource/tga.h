#include "platform.h"


rgbaColor *extractTGAData( unsigned char *inData, int inNumBytes,
                           int *outWidth, int *outHeight );


rgbaColor *readTGAFile( const char *inFileName, int *outWidth, int *outHeight );
