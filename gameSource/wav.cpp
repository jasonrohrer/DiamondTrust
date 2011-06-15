#include "wav.h"
#include <string.h>


static unsigned int bytesToInt( unsigned char inBuffer[4] ) {
    // little endian in a wav file

    return (unsigned int) (
        inBuffer[0] | inBuffer[1] << 8 | 
        inBuffer[2] << 16 | inBuffer[3] << 24 );
    }


static unsigned short bytesToShort( unsigned char inBuffer[2] ) {
    // little endian in a wav file

    return (unsigned short) (
        inBuffer[0] | inBuffer[1] );
    }



FileHandle openWavFile( char *inFilePath, wavInfo *outWavInfo ) {
    
    int fileSize;
    
    unsigned int startMS = getSystemMilliseconds();
        
    FileHandle handle = openFile( inFilePath, &fileSize );
    
    unsigned int netMS = getSystemMilliseconds() - startMS;
    
    if( netMS > 5 ) {
        printOut( "   openFile operation took %dms\n", netMS );
        }

    
    if( handle == NULL ) {
        return NULL;
        }
    
    // check overall header size, so we don't have to check for read failures
    // over and over below
    if( fileSize < 44 ) {
        printOut( "File not big enough to contain basic WAV header, %s\n",
                  inFilePath );
    
        closeFile( handle );
        return NULL;
        }
    

    int bytesLeftInFile = fileSize;
    


    unsigned char chunkBuffer[5];
    // terminate so we can use it as a c-string
    chunkBuffer[4] = '\0';
    

    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    if( strcmp( (char*)chunkBuffer, "RIFF" ) != 0 ) {
        printOut( "RIFF not found at start of WAV file %s\n", inFilePath );
        closeFile( handle );
        return NULL;
        }
    
    // skip chunk size;
    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );


    // look for WAVE

    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    if( strcmp( (char*)chunkBuffer, "WAVE" ) != 0 ) {
        printOut( "WAVE not found at start of WAV file %s\n", inFilePath );
        closeFile( handle );
        return NULL;
        }

    // look for "fmt "


    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    if( strcmp( (char*)chunkBuffer, "fmt " ) != 0 ) {
        printOut( "'fmt ' not found at start of WAV file %s\n", inFilePath );
        closeFile( handle );
        return NULL;
        }

    // skip fmt chunk size (should be 16)
    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );

    if( bytesToInt( chunkBuffer ) != 16 ) {
        printOut( "fmt chunk in WAV file %s has weird length %d\n",
                  inFilePath, bytesToInt( chunkBuffer ) );
        closeFile( handle );
        return NULL;
        }
    

    // 2 bytes of format  1 means PCM
    bytesLeftInFile -= readFile( handle, chunkBuffer, 2 );
    
    if( bytesToShort( chunkBuffer ) != 1 ) {
        printOut( "Non-PCM WAV files cannot be read, %s\n",
                  inFilePath );
        closeFile( handle );
        return NULL;
        }
    


    wavInfo info;
    




    // 2 bytes encoding number of channels
    bytesLeftInFile -= readFile( handle, chunkBuffer, 2 );
    
    info.numChannels = bytesToShort( chunkBuffer );


    // 4 bytes encoding sample rate
    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    info.sampleRate = (int)bytesToInt( chunkBuffer );


    // 4 bytes of ByteRate (ignore)
    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );


    // 2 bytes of block align (ignore)
    bytesLeftInFile -= readFile( handle, chunkBuffer, 2 );


    // 2 bytes of bits per sample
    bytesLeftInFile -= readFile( handle, chunkBuffer, 2 );

    info.bitsPerSample = bytesToShort( chunkBuffer );
    




    // look for data

    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    if( strcmp( (char*)chunkBuffer, "data" ) != 0 ) {
        printOut( "'data' not found at start of WAV file %s\n", inFilePath );
        closeFile( handle );
        return NULL;
        }

    
    // size of data chunk in bytes
    bytesLeftInFile -= readFile( handle, chunkBuffer, 4 );
    
    unsigned int dataLength = bytesToInt( chunkBuffer );
    
    if( bytesLeftInFile < dataLength ) {
        printOut( "Not enough room for data chunk in WAV file %s\n", 
                  inFilePath );
        closeFile( handle );
        return NULL;
        }
    
    int bytesPerFullSample = ( info.bitsPerSample / 8 ) * info.numChannels;
    

    info.numSamples = (int)( dataLength / bytesPerFullSample );

    
    info.startOfDataInFile = fileSize - bytesLeftInFile;
    
    
    *outWavInfo = info;
    
    
    return handle;
    }

