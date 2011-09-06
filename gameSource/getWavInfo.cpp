#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void usage() {
    printf( "Prints the sample rate or length-in-samples of WAV and exits\n\n" 
            "Usage:\n"
            "  getWaveInfo rate inWavFilePath\n\n"
            "  getWaveInfo peak inWavFilePath\n\n"
            "  getWaveInfo length inWavFilePath\n" );
    exit(0);
    }




static unsigned int bytesToInt( unsigned char inBuffer[4] ) {
    // little endian in a wav file

    return (unsigned int) (
        inBuffer[0] | inBuffer[1] << 8 | 
        inBuffer[2] << 16 | inBuffer[3] << 24 );
    }


static unsigned short bytesToUnsignedShort( unsigned char inBuffer[2] ) {
    // little endian in a wav file

    return (unsigned short) (
        inBuffer[0] | inBuffer[1] << 8 );
    }


static unsigned short bytesToShort( unsigned char inBuffer[2] ) {
    // little endian in a wav file

    return (short) (
        inBuffer[0] | inBuffer[1] << 8 );
    }




int main( int inNumArgs, const char **inArgs ) {
    
    if( inNumArgs != 3 ) {
        usage();
        }
    
    char printRate = true;
    char printPeak = false;
    
    if( inArgs[1][0] == 'l' ) {
        // print length instead
        printRate = false;
        }
    else if( inArgs[1][0] == 'p' ) {
        printRate = false;
        printPeak = true;
        }
    


    const char *inFilePath = inArgs[2];


    int fileSize;
    
    FILE *handle = fopen( inFilePath, "rb" );
    
    if( handle == NULL ) {
        printf( "Failed to open file '%s'\n", inFilePath );
        
        return 0;
        }
    
    


    unsigned char chunkBuffer[5];
    // terminate so we can use it as a c-string
    chunkBuffer[4] = '\0';
    

    fread( chunkBuffer, 1, 4, handle );
    
    if( strcmp( (char*)chunkBuffer, "RIFF" ) != 0 ) {
        printf( "RIFF not found at start of WAV file %s\n", inFilePath );
        fclose( handle );
        return 0;
        }
    
    // skip chunk size;
    fread( chunkBuffer, 1, 4, handle );
    

    // look for WAVE
    fread( chunkBuffer, 1, 4, handle );

    if( strcmp( (char*)chunkBuffer, "WAVE" ) != 0 ) {
        printf( "WAVE not found at start of WAV file %s\n", inFilePath );
        fclose( handle );
        return 0;
        }

    // look for "fmt "
    fread( chunkBuffer, 1, 4, handle );

    if( strcmp( (char*)chunkBuffer, "fmt " ) != 0 ) {
        printf( "'fmt ' not found at start of WAV file %s\n", inFilePath );
        fclose( handle );
        return 0;
        }

    // skip fmt chunk size (should be 16)
    fread( chunkBuffer, 1, 4, handle );

    if( bytesToInt( chunkBuffer ) != 16 ) {
        printf( "fmt chunk in WAV file %s has weird length %d\n",
                  inFilePath, bytesToInt( chunkBuffer ) );
        fclose( handle );
        return 0;
        }
    

    // 2 bytes of format  1 means PCM
    fread( chunkBuffer, 1, 2, handle );
    
    if( bytesToUnsignedShort( chunkBuffer ) != 1 ) {
        printf( "Non-PCM WAV files cannot be read, %s\n",
                  inFilePath );
        fclose( handle );
        return 0;
        }
    


    // 2 bytes encoding number of channels
    fread( chunkBuffer, 1, 2, handle );
    
    unsigned short numChannels = bytesToUnsignedShort( chunkBuffer );


    // 4 bytes encoding sample rate
    fread( chunkBuffer, 1, 4, handle );
        
    int sampleRate = (int)bytesToInt( chunkBuffer );


    if( printRate ) {
        printf( "%d", sampleRate );
        }
    
    
    // 4 bytes of ByteRate (ignore)
    fread( chunkBuffer, 1, 4, handle );
    

    // 2 bytes of block align (ignore)
    fread( chunkBuffer, 1, 2, handle );
    

    // 2 bytes of bits per sample
    fread( chunkBuffer, 1, 2, handle );

    int bitsPerSample = bytesToUnsignedShort( chunkBuffer );



    // look for data
    fread( chunkBuffer, 1, 4, handle );

    if( strcmp( (char*)chunkBuffer, "data" ) != 0 ) {
        printf( "'data' not found at start of WAV file %s\n", inFilePath );
        fclose( handle );
        return 0;
        }

    
    // size of data chunk in bytes
    fread( chunkBuffer, 1, 4, handle );
    
    unsigned int dataLength = bytesToInt( chunkBuffer );
    
    
    int bytesPerFullSample = ( bitsPerSample / 8 ) * numChannels;
    

    int numSamples = (int)( dataLength / bytesPerFullSample );

    
    if( !printRate && !printPeak ) {
        printf( "%d", numSamples );
        }


    if( printPeak ) {
    
        // walk through whole file and find peak sample    
        
        unsigned char *dataBuffer = new unsigned char[ dataLength ];
        
        if( dataLength != fread( dataBuffer, 1, dataLength, handle ) ) {
            printf( "Failed to read full %d bytes of data from WAV file %s\n",
                    inFilePath );
            
            fclose( handle );
            return 0;
            }
        

        int bytesPerSingleSample = bytesPerFullSample / numChannels;
        
        int numSingleSamples = dataLength / bytesPerSingleSample;

        short maxSample = 0;

        for( int s=0; s<numSingleSamples; s++ ) {
        
            short sample = 
                bytesToShort( &( dataBuffer[ s * bytesPerSingleSample ] ) );
            
            if( sample < 0 ) {
                sample *= -1;
                }
            
            if( sample > maxSample ) {
                maxSample = sample;
                }
            }
        
        printf( "%d", maxSample );


        delete [] dataBuffer;
        }
    


    fclose( handle );

    return 0;
    }
