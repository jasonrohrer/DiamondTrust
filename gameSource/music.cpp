#include "platform.h"

#include "music.h"
#include "wav.h"


wavInfo info;
    
FileHandle wavHandle;

void initMusic() {
    wavHandle = openWavFile( "gong.wav", &info );

    if( wavHandle != NULL ) {
        
        if( info.numChannels != 1 || info.bitsPerSample != 16 ) {
            printOut( "Only mono, 16-bit WAV files supported\n" );
            printOut( "(not  %d chan, %d-bit, %d rate, %d samples )\n",
                  info.numChannels, info.bitsPerSample, 
                  info.sampleRate, info.numSamples );
            closeFile( wavHandle );
            wavHandle = NULL;
            }
        }
    
    
    }



void freeMusic() {
    if( wavHandle != NULL ) {
        closeFile( wavHandle );
        }
    }




static int sampleIndices[16] = { 0 };

#include "wihaho.pcm16.c"

void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {
    // for now, return silence

    if( inChannelNumber == 0 && wavHandle != NULL ) { 
        
   
        int sampleIndex = sampleIndices[inChannelNumber];

        unsigned char oneSampleBuffer[2];

        for( int i=0; i<inNumSamples; i++ ) {
            
            readFile( wavHandle, oneSampleBuffer, 2 );
            
            inBuffer[i] = oneSampleBuffer[0] | oneSampleBuffer[1] << 8;
            

            sampleIndex += 1;
            
            // wrap
            if( sampleIndex >= info.numSamples ) {
                sampleIndex = 0;
                
                // re-open wav file
                closeFile( wavHandle );
                wavHandle = openWavFile( "gong.wav", &info );
                }


            //inBuffer[i] = 0;
            }
        sampleIndices[0] = sampleIndex;
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
