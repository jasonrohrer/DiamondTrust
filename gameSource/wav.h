

#include "platform.h"


#include <stdio.h>


typedef struct wavInfo {
        int numChannels;
        int bitsPerSample;
        int sampleRate;
        int numSamples;

        // number of bytes to skip in the file to get to the data
        int startOfDataInFile;

    } wavInfo;



// opens a WAV file and skips the file pointer to the start of samples
FileHandle openWavFile( char *inFilePath, wavInfo *outWavInfo );
