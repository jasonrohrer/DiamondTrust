

#include "platform.h"


typedef struct wavInfo {
        int numChannels;
        int bitsPerSample;
        int sampleRate;
        int numSamples;
    } wavInfo;



// opens a WAV file and skips the file pointer to the start of samples
FileHandle openWavFile( char *inFilePath, wavInfo *outWavInfo );
