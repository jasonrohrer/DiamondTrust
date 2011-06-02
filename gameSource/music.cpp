

#include "platform.h"



void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {
    // for now, return silence

    for( int i=0; i<inNumSamples; i++ ) {
        inBuffer[i] = 0;
        }
    }
