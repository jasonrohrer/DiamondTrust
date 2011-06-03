

#include "platform.h"



static int sampleIndices[16] = { 0 };

#include "wihaho.pcm16.c"

void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {
    // for now, return silence

    if( inChannelNumber == 0 ) {    
        for( int i=0; i<inNumSamples; i++ ) {
            inBuffer[i] = 0;
            }
        }
    

    /*
    if( false && inChannelNumber == 0 ) {
        // something sound-like in first channel
    
        int sampleIndex = sampleIndices[0];

        for( int i=0; i<inNumSamples; i++ ) {
            inBuffer[i] = wihaho_pcm16[sampleIndex];
            sampleIndex++;

            // wrap
            if( sampleIndex == WIHAHO_PCM16_LOOPLEN * 2 ) {
                sampleIndex = 0;
                }
            }
        sampleIndices[0] = sampleIndex;
        }
    

    if( false && inChannelNumber == 1 ) {
        // something sound-like in first channel
    
        int sampleIndex = sampleIndices[1];

        for( int i=0; i<inNumSamples; i++ ) {
            inBuffer[i] = wihaho_pcm16[sampleIndex];
            
            // double rate
            sampleIndex += 2;

            // wrap
            if( sampleIndex >= WIHAHO_PCM16_LOOPLEN ) {
                sampleIndex = 0;
                }
            }
        sampleIndices[1] = sampleIndex;
        }
    */
    if( inChannelNumber > 0 ) {
        // something sound-like in first channel
    
        int sampleIndex = sampleIndices[inChannelNumber];

        for( int i=0; i<inNumSamples; i++ ) {
            inBuffer[i] = wihaho_pcm16[sampleIndex];
            
            // 1/c rate
            if( i%inChannelNumber == 0 ) {
                sampleIndex += 1;
                }
            
            // wrap
            if( sampleIndex >= WIHAHO_PCM16_LOOPLEN ) {
                sampleIndex = 0;
                }
            }
        sampleIndices[inChannelNumber] = sampleIndex;
        }


    /*
    if( inChannelNumber == 3 ) {
        // something sound-like in first channel
    
        int sampleIndex = sampleIndices[3];

        for( int i=0; i<inNumSamples; i++ ) {
            inBuffer[i] = wihaho_pcm16[sampleIndex];
            
            // 1/3 rate
            if( i%3 == 0 ) {
                sampleIndex += 1;
                }
            
            // wrap
            if( sampleIndex >= WIHAHO_PCM16_LOOPLEN ) {
                sampleIndex = 0;
                }
            }
        sampleIndices[3] = sampleIndex;
        }
    */
    }
