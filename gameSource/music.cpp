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


int gridStepLength = 0;



typedef struct channelStream {
        
        // false if channel is silent
        char filePlaying;
        
        unsigned int totalNumSamplesPlayed;


        char *wavFileName;
        
        FileHandle wavFile;
        wavInfo info;
        
        int fileSamplePosition;

        int startSampleDelay;
        
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



static void sortStrings( char ***inArray, int inNumStrings ) {
    
    char **array = *( inArray );
    
    // slow bubble sort, but it doesn't matter for such small sets

    
    char sorted = false;
    
    while( ! sorted ) {
        sorted = true;

        for( int i=0; i<inNumStrings-1; i++ ) {
            
            if( strcmp( array[i], array[i+1] ) > 0 ) {
                
                // swap
                char *temp = array[i+1];
            
                array[i+1] = array[i];
            
                array[i] = temp;
                sorted = false;
                }
            }
        }
    }






void initMusic() {
    // all channels start not playing
    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        s->wavFileName = NULL;
        s->wavFile = NULL;
        s->filePlaying = false;
        s->totalNumSamplesPlayed = 0;
        }



    // pick a song at random

    int numSongs;
    
    char **songList = listDirectory( "music", &numSongs );
    
    if( numSongs > 0 ) {
        int songPick = getRandom( numSongs );

        currentSongDirName = stringDuplicate( songList[ songPick ] );

        
        
        songActDirNames = listDirectory( currentSongDirName, 
                                         &numSongActs );

        
        

        currentSongAct = 0;
        
        if( numSongActs > 1 ) {
            sortStrings( &songActDirNames, numSongActs );
                    
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
                    replaceOnce( songPartDirNames[p], path, "", &found );
                
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
                delete [] songPartDirNames[p];
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




void addTrack( int inChannelNumber, int inDelay ) {
    
    int partPick = inChannelNumber;
    
    char *actDir = songActDirNames[ currentSongAct ];
    char *partName = songPartNames[ partPick ];
                    
    char *partDir = autoSprintf( "%s/%s", actDir, partName );
                    
    if( isDirectory( partDir ) ) {
                        
        int numPartFiles;
        char **partFiles = 
            listDirectory( partDir, &numPartFiles );
                        
        
        // also consider files for this part that are in first act
        // (they can be used throughout song)
        
        char *firstPartDir = autoSprintf( "%s/%s", 
                                          songActDirNames[0], partName );
        
        if( isDirectory( firstPartDir ) ) {
            // add into mix of possible parts to chose from
            
            SimpleVector<char *> fullPartFileVector;
            
            fullPartFileVector.appendArray( partFiles, numPartFiles );
            
            int numFirstPartFiles;
            
            char **firstPartFiles = listDirectory( firstPartDir,
                                                   &numFirstPartFiles );
            
            if( numFirstPartFiles > 0 ) {
                fullPartFileVector.appendArray( firstPartFiles, 
                                                numFirstPartFiles );
                
                }
            delete [] firstPartFiles;
            
            delete [] partFiles;
            
            partFiles = fullPartFileVector.getElementArray();
            numPartFiles = fullPartFileVector.size();
            }
            


        if( numPartFiles > 0 ) {
                            
            int partFilePick = 
                getRandom( numPartFiles );
                            
                            
            channelStream *s = &( songStreams[partPick] );
                            
            s->wavFileName = 
                stringDuplicate( partFiles[ partFilePick ] );
                            
            s->wavFile = 
                openWavFile( s->wavFileName, &( s->info ) );
            s->fileSamplePosition = 0;
            s->startSampleDelay = inDelay;
            s->filePlaying = true;

            printOut( "Adding loop %s on channel %d with delay %d\n", 
                      s->wavFileName, partPick, inDelay );

                            
            if( s->info.numSamples > gridStepLength ) {
                // an even longer loop encountered
                // use this as our grid step

                gridStepLength = s->info.numSamples;
                printOut( "New longer grid step discovered:  %d\n", 
                          gridStepLength );
                }

            }
        deleteArrayOfStrings( &partFiles, numPartFiles );
        }
    else {
        printOut( "Part %s not found in song act %s\n",
                  partName, actDir );
        }
                    
    delete [] partDir;
    }







void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {

    int numSamplesRequested = inNumSamples;


    channelStream *s = &( songStreams[inChannelNumber] );


    if( ! s->filePlaying && inChannelNumber < numSongParts ) {
    
        if( gridStepLength == 0
            ||
            s->totalNumSamplesPlayed / gridStepLength <
            ( s->totalNumSamplesPlayed + inNumSamples ) / gridStepLength ) {
        
            // a grid step occurs in THIS buffer
    
            
            int delay = 0;
            if( gridStepLength > 0 ) {
                delay = gridStepLength - 
                    ( s->totalNumSamplesPlayed % gridStepLength );
                }
            
            // consider adding a new track
            if( getRandom( 100 ) > 10 ) {
                addTrack( inChannelNumber, delay );                
                }
            }
        
        }
    



    if( s->filePlaying ) {
        //printOut( "%d playing %s\n", inChannelNumber, s->wavFileName );
        
        // keep looping through file until we've filled the requested buffer
        while( inNumSamples > 0 ) {

            if( s->startSampleDelay > 0 ) {
                
                // stick more silence in before this loop actually starts 
                // playing
                
                int numToGet = inNumSamples;
                
                if( numToGet > s->startSampleDelay ) {
                    numToGet = s->startSampleDelay;
                    }
                
                memset( inBuffer, 0, numToGet * 2 );
                
                s->startSampleDelay -= numToGet;
            
                inBuffer = &( inBuffer[numToGet] );
            
            
                inNumSamples -= numToGet;
                }
            
            
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
                s->wavFile = NULL;
                delete [] s->wavFileName;

                // consider dropping out
                if( getRandom( 100 ) < 50 ) {
                    printOut( "Track %d dropping out\n", inChannelNumber );
                    
                    s->filePlaying = false;

                    // fill rest with silence
                    memset( inBuffer, 0, inNumSamples * 2 );
                    inNumSamples = 0;
                    }
                else {
                    // consider switching to a different loop for this part

                    // no delay, because it can start right NOW
                    addTrack( inChannelNumber, 0 );
                    }
                
                
                //s->wavFile = openWavFile( s->wavFileName, &( s->info ) );
                }
            }
        }
    else {
        // silence
        memset( inBuffer, 0, inNumSamples * sizeof( s16 ) );
        
        }

    s->totalNumSamplesPlayed += numSamplesRequested;
    
    if( inChannelNumber == 0 ) {
        //printOut( "Total played %d\n", s->totalNumSamplesPlayed );
        }
    
    return;

    }
