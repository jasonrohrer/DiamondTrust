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


typedef struct wavStream {
        char *wavFileName;
        
        FileHandle wavFile;
        wavInfo info;
    } wavStream;


// for performance, keep all possible wav files open and ready to go
// so they are opened and buffered all the time
SimpleVector<wavStream> wavBank;


wavStream *findBankStreamByFileName( char *inWavFileName ) {
    for( int i=0; i<wavBank.size(); i++ ) {
        wavStream *s = wavBank.getElement( i );
    
        if( strcmp( s->wavFileName, inWavFileName ) == 0 ) {
            return s;
            }
        }
    
    return NULL;
    }



void rewindWavBankStream( wavStream *inStream ) {
    fileSeek( inStream->wavFile, inStream->info.startOfDataInFile );
    }





typedef struct channelStream {
        
        // false if channel is silent
        char filePlaying;
        
        unsigned int totalNumSamplesPlayed;

        // a pointer to a wavStream in our wavBank
        wavStream *wavBankStream;
        
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

        s->wavBankStream = NULL;
        
        s->filePlaying = false;
        s->totalNumSamplesPlayed = 0;
        }


    // no music on clones
    // but at least init all channels to not playing above
    if( isThisAClone() ) {
        return;
        }



    // pick a song at random

    int numSongs;
    
    char **songList = listDirectory( "music", &numSongs );
    
    if( numSongs > 0 ) {
        int songPick = (int)getRandom( (unsigned int)numSongs );

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



                
                // add all WAV files in this part to our streamBank
                // open them and get them ready to go
                
                
                int numWavFiles;
                char **wavFileNames = listDirectory( songPartDirNames[p],
                                                     &numWavFiles );
                
                for( int w=0; w<numWavFiles; w++ ) {
                    char *wavName = wavFileNames[w];

                    int nameLength = strlen( wavName );
                    
                    if( nameLength > 4 ) {
                        char *suffix = &( wavName[ nameLength - 4 ] );
                    
                        if( strcmp( suffix, ".wav" ) == 0 ) {
                            // a wav file


                            wavStream stream;
                            
                            stream.wavFileName = 
                                stringDuplicate( wavFileNames[w] );
                            

                            stream.wavFile =
                                openWavFile( stream.wavFileName, 
                                             &( stream.info ) );

                            
                            wavBank.push_back( stream );
                            }
                        }
                    
                    
                    delete [] wavFileNames[w];
                    }
                delete [] wavFileNames;

                





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
        

        printOut( "\n\nThe following WAV files have been opened and "
                  "are ready to go:\n" );
        
        for( int i=0; i<wavBank.size(); i++ ) {
            wavStream *s = wavBank.getElement( i );

            printOut( "  %s\n", s->wavFileName );
            }
        
        printOut( "\n\n" );
        

        }
    else {
        printOut( "ERROR:  no songs present!\n" );
        }

    deleteArrayOfStrings( &songList, numSongs );
    }






void freeMusic() {

    // no music on clones
    if( isThisAClone() ) {
        return;
        }


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
            s->wavBankStream = NULL;
            
            s->filePlaying = false;
            }        
        }
    

    for( int i=0; i<wavBank.size(); i++ ) {
        wavStream *s = wavBank.getElement( i );
        
        closeFile( s->wavFile );
        
        delete [] s->wavFileName;
        }
    wavBank.deleteAll();

    }




static void addTrack( int inChannelNumber, int inDelay ) {
    
    int partPick = inChannelNumber;
    
    char *actDir = songActDirNames[ currentSongAct ];
    char *partName = songPartNames[ partPick ];
                    
    char *partDir = autoSprintf( "%s/%s", actDir, partName );


    SimpleVector<char *> fullPartFileVector;

                    
    if( isDirectory( partDir ) ) {
                        
        int numPartFiles;
        char **partFiles = 
            listDirectory( partDir, &numPartFiles );
                        
        
        if( numPartFiles > 0 ) {
            fullPartFileVector.appendArray( partFiles, numPartFiles );
            }
        
        delete [] partFiles;
        }
    delete [] partDir;

    
    
    // also consider files for this part that are in first act
    // (they can be used throughout song)
    
    char *firstPartDir = autoSprintf( "%s/%s", 
                                      songActDirNames[0], partName );
        
    if( isDirectory( firstPartDir ) ) {
        // add into mix of possible parts to chose from
                    
        int numFirstPartFiles;
        
        char **firstPartFiles = listDirectory( firstPartDir,
                                               &numFirstPartFiles );
            
        if( numFirstPartFiles > 0 ) {
            fullPartFileVector.appendArray( firstPartFiles, 
                                            numFirstPartFiles );        
            }
        
        delete [] firstPartFiles;
        }
        
    delete [] firstPartDir;
    
    char **partFiles = fullPartFileVector.getElementArray();
    int numPartFiles = fullPartFileVector.size();
            


    if( numPartFiles > 0 ) {
                            
        int partFilePick = (int)getRandom( (unsigned int)numPartFiles );
                            
        
        channelStream *s = &( songStreams[partPick] );

        // already rewound if it's not assigned to a channel
        s->wavBankStream = 
            findBankStreamByFileName( partFiles[ partFilePick ] );

        s->fileSamplePosition = 0;
        s->startSampleDelay = inDelay;
        s->filePlaying = true;
        
        printOut( "Adding loop %s on channel %d with delay %d\n", 
                  s->wavBankStream->wavFileName, partPick, inDelay );
        
                            
        if( s->wavBankStream->info.numSamples > gridStepLength ) {
            // an even longer loop encountered
            // use this as our grid step
            
            gridStepLength = s->wavBankStream->info.numSamples;
            printOut( "New longer grid step discovered:  %d\n", 
                      gridStepLength );
            }

        }
    else {
        printOut( "Part %s not found in song act %s (or in base act)\n",
                  partName, actDir );
        
        songStreams[partPick].filePlaying = false;
        }

    deleteArrayOfStrings( &partFiles, numPartFiles );                    
    }







void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {

    int numSamplesRequested = inNumSamples;


    channelStream *s = &( songStreams[inChannelNumber] );


    if( ! s->filePlaying && inChannelNumber < numSongParts ) {
    
        if( gridStepLength == 0
            ||
            s->totalNumSamplesPlayed == 0
            ||
            s->totalNumSamplesPlayed / gridStepLength <
            ( s->totalNumSamplesPlayed + inNumSamples ) / gridStepLength ) {
        
            // a grid step occurs in THIS buffer
    
            
            int delay = 0;
            if( gridStepLength > 0 && s->totalNumSamplesPlayed > 0 ) {
                delay = (int)( 
                    gridStepLength - 
                    ( s->totalNumSamplesPlayed % gridStepLength ) );
                }
            
            // consider adding a new track
            if( getRandom( 100 ) > 30 ) {
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
                
                memset( inBuffer, 0, (unsigned int)( numToGet * 2 ) );
                
                s->startSampleDelay -= numToGet;
            
                inBuffer = &( inBuffer[numToGet] );
            
            
                inNumSamples -= numToGet;
                }
            
            
            int numToGet = inNumSamples;

            int numSamplesLeft = s->wavBankStream->info.numSamples - 
                s->fileSamplePosition;
            
            if( numToGet > numSamplesLeft ) {
                numToGet = numSamplesLeft;
                }
            
            unsigned int startMS = getSystemMilliseconds();

            readFile( s->wavBankStream->wavFile, 
                      (unsigned char *)inBuffer, numToGet * 2 );
            
            unsigned int netMS = getSystemMilliseconds() - startMS;
        
            if( netMS > 5 ) {
                printOut( "Reading from wav file for channel %d took %dms\n",
                          inChannelNumber, netMS );
                }


            s->fileSamplePosition += numToGet;
            
            inBuffer = &( inBuffer[numToGet] );
            
            
            inNumSamples -= numToGet;
            
            
            if( s->fileSamplePosition == s->wavBankStream->info.numSamples ) {
                // at end of file, wrap back
                s->fileSamplePosition = 0;
                
                unsigned int startMS = getSystemMilliseconds();
                
                rewindWavBankStream( s->wavBankStream );
                
                unsigned int netMS = getSystemMilliseconds() - startMS;
                
                if( netMS > 5 ) {
                    printOut( "Rewinding wav file for channel %d took %dms\n",
                              inChannelNumber, netMS );
                    }


                s->wavBankStream = NULL;

                // consider dropping out
                if( getRandom( 100 ) < 50 ) {
                    printOut( "Track %d dropping out\n", inChannelNumber );
                    
                    s->filePlaying = false;

                    // fill rest with silence
                    memset( inBuffer, 0, (unsigned int)( inNumSamples * 2 ) );
                    inNumSamples = 0;
                    }
                else {
                    // consider switching to a different loop for this part

                    // no delay, because it can start right NOW
                    addTrack( inChannelNumber, 0 );

                    if( ! s->filePlaying ) {
                        // adding a track on this channel failed?
                        // maybe an act change has happened and we no
                        // longer have loops to play on this channel
                        printOut( 
                            "Track %d dropping out because of act change\n", 
                            inChannelNumber );
                        
                        // fill rest with silence
                        memset( inBuffer, 
                                0, (unsigned int)( inNumSamples * 2 ) );
                        
                        inNumSamples = 0;
                        }
                    
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




void nextSongAct() {    
    lockAudio();
    if( currentSongAct < numSongActs - 1 ) {
        currentSongAct ++;
        }
    unlockAudio();
    }



int getSongAct() {    
    int returnValue;
    
    lockAudio();
    returnValue = currentSongAct;
    unlockAudio();
    
    return returnValue;
    }




char **getTrackInfoStrings( int *outNumTracks ) {
    char **strings = new char*[MAX_SOUND_CHANNELS];
    
    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        if( s->filePlaying ) {
            strings[i] = autoSprintf( 
                "%d:  %s (%d.%d/%d.%d)", i, s->wavBankStream->wavFileName,
                s->fileSamplePosition / 22050, 
                s->fileSamplePosition / 2205 - 
                  10 * ( s->fileSamplePosition / 22050 ), 
                s->wavBankStream->info.numSamples / 22050,
                s->wavBankStream->info.numSamples / 2205 - 
                  10 * ( s->wavBankStream->info.numSamples / 22050 ) );
            }
        else {
            strings[i] = autoSprintf( "%d:  OFF", i );
            }
        
        }
    
    *outNumTracks = MAX_SOUND_CHANNELS;

    return strings;
    }




