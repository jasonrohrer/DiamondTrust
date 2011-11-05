
#include <stdio.h>
#include <stdlib.h>


static void usage() {
    printf( "\nUsage:\n"
            "replaceLogo oldRom.srl newRom.srl logoRom.nds\n\n"
            "logoRom.nds must be ROM created by ndstool that contains a\n"
            "compressed and CRD'd logo.\n"
            "(build an ndstool ROM using a .BMP as the logo)\n\n" );
    exit( 0 );
    }




int main( int inNumArgs, const char **inArgs ) {
    
    if( inNumArgs != 4 ) {
        usage();
        }
    
    FILE *oldRom = fopen( inArgs[1], "rb" );
    FILE *newRom = fopen( inArgs[2], "wb" );
    FILE *logoRom = fopen( inArgs[3], "rb" );
    

    // copy first 192 bytes blindly (before logo)

    unsigned char buffer[5000];
    
    fread( buffer, 1, 192, oldRom );
    
    fwrite( buffer, 1, 192, newRom );
    

    // discard log ROM's first 192 bytes
    fread( buffer, 1, 192, logoRom );

    
    // now REPLACE next 156 bytes logo bytes and 2 bytes of logo CRC

    // discard these from old rom
    fread( buffer, 1, 158, oldRom );

    fread( buffer, 1, 158, logoRom );
    
    fwrite( buffer, 1, 158, newRom );
    


    fclose( logoRom );
    


    
    int numRead = 5000;
    

    // copy rest over in blocks of 5000

    while( numRead == 5000 ) {
        numRead = fread( buffer, 1, 5000, oldRom );
        if( numRead > 0 ) {
            fwrite( buffer, 1, numRead, newRom );
            }
        }
    fclose( oldRom );
    fclose( newRom );
    

    
    return 0;
    }
