#include "platform.h"

#include "music.h"
#include "wav.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"



char *currentSongDirName = NULL;


int numSongActs;
char **songActDirNames = NULL;

int currentSongAct;


int numSongParts;
char **songPartNames = NULL;



typedef struct channelStream {
        
        // false if channel is silent
        char filePlaying;
        

        char *wavFileName;
        
        FileHandle wavFile;
        wavInfo info;
        
        int fileSamplePosition;
    } channelStreaml;
        


channelStream songStreams[ MAX_SOUND_CHANNELS ];




static void deleteArrayOfStrings( char ***inArray, int inNumStrings ) {
    
    char **array = *( inArray );

    for( int i=0; i<inNumStrings; i++ ) {
        delete [] array[i];
        }
    delete [] array;
    
    *inArray = NULL;
    }





void initMusic() {
    // all channels start not playing
    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        s->wavFileName = NULL;
        s->wavFile = NULL;
        s->filePlaying = false;
        }



    // pick a song at random

    int numSongs;
    
    char **songList = listDirectory( "music", &numSongs );
    
    if( numSongs > 0 ) {
        int songPick = getRandom( numSongs - 1 );

        currentSongDirName = stringDuplicate( songList[ songPick ] );

        
        
        char **songActDirNames = listDirectory( currentSongDirName, 
                                                &numSongActs );

        currentSongAct = 0;
        
        if( numSongActs > 1 ) {
            // first act is common tracks for all acts
            currentSongAct = 1;
            }

        

        printOut( "Available song parts:\n" );
        
        // look through all acts and get union of all possible part names
        SimpleVector<char *> allPartNames;
        
        for( int i=0; i<numSongActs; i++ ) {
            
            
            int numParts;
            
            char **songPartDirNames = listDirectory( songActDirNames[i], 
                                                     &numParts );
            
            for( int p=0; p<numParts; p++ ) {

                char *path = autoSprintf( "%s/", songActDirNames[i] );
                
                char found;
                
                char *songPartName = 
                    replaceOnce( songPartDirNames[i], path, "", &found );
                
                delete [] path;
                
                // check if it's already present in vector

                char hit = false;
                for( int q=0; q<allPartNames.size(); q++ ) {
                
                    if( strcmp( *( allPartNames.getElement(q) ),
                                songPartName ) == 0 ) {
                        hit = true;
                        break;
                        }                    
                    }
                
                if( !hit ) {
                    allPartNames.push_back( stringDuplicate( songPartName ) );

                    printOut( "  %s\n", songPartName );
                    }                

                delete [] songPartName;
                delete [] songPartDirNames;
                }
            delete [] songPartDirNames;            
            }
        
        numSongParts = allPartNames.size();
        songPartNames = allPartNames.getElementArray();

        if( numSongParts > MAX_SOUND_CHANNELS ) {
            printOut( "Warning:  more parts than available channels\n" );
            }

        }
    else {
        printOut( "ERROR:  no songs present!\n" );
        }

    deleteArrayOfStrings( &songList, numSongs );
    }






void freeMusic() {

    if( currentSongDirName != NULL ) {
        delete [] currentSongDirName;
        }
    
    if( songActDirNames != NULL ) {
        deleteArrayOfStrings( &songActDirNames, numSongActs );
        }


    if( songPartNames != NULL ) {
        deleteArrayOfStrings( &songPartNames, numSongParts );
        }



    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        if( s->filePlaying  ) {
            closeFile( s->wavFile );
            s->wavFile = NULL;
            s->filePlaying = false;

            delete [] s->wavFileName;
            s->wavFileName = NULL;
            }        
        }
    


    }




static int sampleIndices[16] = { 0 };

#include "wihaho.pcm16.c"

void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {


    channelStream *s = &( songStreams[inChannelNumber] );

    if( s->filePlaying ) {
        
        // keep looping through file until we've filled the requested buffer
        while( inNumSamples > 0 ) {
            
            int numToGet = inNumSamples;

            int numSamplesLeft = s->info.numSamples - s->fileSamplePosition;
            
            if( numToGet > numSamplesLeft ) {
                numToGet = numSamplesLeft;
                }
            
            readFile( s->wavFile, (unsigned char *)inBuffer, numToGet * 2 );
            
            s->fileSamplePosition += numToGet;
            
            inBuffer = &( inBuffer[numToGet] );
            
            
            inNumSamples -= numToGet;
            
            
            if( s->fileSamplePosition == s->info.numSamples ) {
                // at end of file, wrap back
                s->fileSamplePosition = 0;
                
                closeFile( s->wavFile );
                s->wavFile = openWavFile( s->wavFileName, &( s->info ) );
                }
            }
        }
    else {
        // silence
        memset( inBuffer, 0, inNumSamples * sizeof( s16 ) );

        
        // consider adding a new track

        if( getRandom( 100 ) > 95 ) {
            printOut( "Adding a new track to the mix\n" );
            
            int partPick = getRandom( numSongParts - 1 );
            
            if( partPick < MAX_SOUND_CHANNELS ) {
                // room for this part

                if( ! songStreams[ partPick ].filePlaying ) {
                    // empty channel waiting
                    
                    char *actDir = songActDirNames[ currentSongAct ];
                    char *partName = songPartNames[ partPick ];
                    
                    char *partDir = autoSprintf( "%s/%s", actDir, partName );
                    
                    if( isDirectory( partDir ) ) {
                        
                        int numPartFiles;
                        char **partFiles = 
                            listDirectory( partDir, &numPartFiles );
                        
                        if( numPartFiles > 0 ) {
                            
                            int partFilePick = getRandom( numPartFiles - 1 );
                            
                            
                            channelStream *s = &( songStreams[partFilePick] );
                            
                            s->wavFileName = 
                                stringDuplicate( partFiles[ partFilePick ] );
                            
                            s->wavFile = 
                                openWavFile( s->wavFileName, &( s->info ) );
                            s->fileSamplePosition = 0;
                            s->filePlaying = true;
                            }
                        deleteArrayOfStrings( &partFiles, numPartFiles );
                        }
                    else {
                        printOut( "Part %s not found in song act %s\n",
                                  partName, actDir );
                        }
                    
                    delete [] partDir;
                    }
                }
            }
        }
    


    return;
    


    /*

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
    */    

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
