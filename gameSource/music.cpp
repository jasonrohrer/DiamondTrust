#include "platform.h"

#include "music.h"
#include "wav.h"
#include "crcHash.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"


// for testing
static char forceFiveTracks = false;



static char limitTotalTracks = false;





int currentSongPick = -1;

char *currentSongDirName = NULL;


unsigned int currentSongTargetLength = 0;


char songSwitchPending = false;


unsigned int rate = SOUND_SAMPLE_RATE;

unsigned int oneTenthRate = rate/10;


static unsigned int songEndFadeTime = rate * 5;




int numSongActs;
char **songActDirNames = NULL;

int currentSongAct = 0;


int numSongParts;
char **songPartNames = NULL;


// songActFilesPerPart[a][p] = number of available files for part p in act a
int **songActFilesPerPart = NULL;

// the full file paths of ALL files (including base act-0 files)
// songActPartFileBanks[a][p] = an array of 
//   strings of length songActFilesPerPart[a][p]
char ****songActPartFileBanks = NULL;


// musicState[p] tells what file index # part p should be playing
// during act changes, there may not actually be enough files for the part
//  to accomodate this file index.  If so, it should wrap around.
//
// -1 for a silent track
int musicState[MAX_SOUND_CHANNELS];

char *lastStateString = NULL;




int gridStepLength = 0;


typedef struct wavStream {
        char *wavFileName;
        
        FileHandle wavFile;
        wavInfo info;
    } wavStream;


// for performance, keep all possible wav files open and ready to go
// so they are opened and buffered all the time
SimpleVector<wavStream> wavBank;


static wavStream *findBankStreamByFileName( char *inWavFileName ) {
    for( int i=0; i<wavBank.size(); i++ ) {
        wavStream *s = wavBank.getElement( i );
    
        if( strcmp( s->wavFileName, inWavFileName ) == 0 ) {
            return s;
            }
        }
    
    return NULL;
    }



static void rewindWavBankStream( wavStream *inStream ) {
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






// returns new array of new strings destroyed by caller
static char **getAllFilesForActAndPart( int inAct, int inPart, 
                                        int *outNumFiles ) {
    
    int partPick = inPart;
    
    char *actDir = songActDirNames[ inAct ];
    char *partName = songPartNames[ partPick ];
                    
    char *partDir = autoSprintf( "%s/%s", actDir, partName );


    SimpleVector<char *> fullPartFileVector;

                    
    if( isDirectory( partDir ) ) {
                        
        int numPartFiles;
        char **partFiles = 
            listDirectory( partDir, &numPartFiles );
                        
        
        if( numPartFiles > 0 ) {
            sortStrings( &partFiles, numPartFiles );
            
            fullPartFileVector.appendArray( partFiles, numPartFiles );
            }
        
        delete [] partFiles;
        }
    delete [] partDir;

    
    


    // also consider files for this part that are in first act
    // (they can be used throughout song)

    // but DON'T do this if inAct IS the first act (common act 0)
    // (don't want doubles)

    if( inAct != 0 ) {
        
    
        char *firstPartDir = autoSprintf( "%s/%s", 
                                          songActDirNames[0], partName );
        
        if( isDirectory( firstPartDir ) ) {
            // add into mix of possible parts to chose from
                    
            int numFirstPartFiles;
        
            char **firstPartFiles = listDirectory( firstPartDir,
                                                   &numFirstPartFiles );
            
            if( numFirstPartFiles > 0 ) {
                sortStrings( &firstPartFiles, numFirstPartFiles );

                fullPartFileVector.appendArray( firstPartFiles, 
                                                numFirstPartFiles );        
                }
        
            delete [] firstPartFiles;
            }
        
        delete [] firstPartDir;
        }
    
    
    *outNumFiles = fullPartFileVector.size();


    return fullPartFileVector.getElementArray();
    }





// picks a song at random
// tries to avoid last song as set in currentSongPick
static void loadSong() {

    // each song may have a different master grid step length
    gridStepLength = 0;
    

    // pick a song at random

    int numSongs;
    
    char **songList = listDirectory( "music", &numSongs );
    
    if( numSongs > 0 ) {
        sortStrings( &songList, numSongs );
        
        int lastSongPick = currentSongPick;

        currentSongPick = (int)getRandom( (unsigned int)numSongs );
        
        if( numSongs > 1 ) {
            // avoid playing the same song again, if possible
            while( currentSongPick == lastSongPick ) {
                
                currentSongPick = (int)getRandom( (unsigned int)numSongs );
                }
            }
        
                
        currentSongDirName = stringDuplicate( songList[ currentSongPick ] );

        
        
        songActDirNames = listDirectory( currentSongDirName, 
                                         &numSongActs );

        if( numSongActs > 1 ) {
            sortStrings( &songActDirNames, numSongActs );
            }
        
        
        if( currentSongAct == 0 ) {
            if( numSongActs > 1 ) {
                // first act is common tracks for all acts
                currentSongAct = 1;
                }
            }
        else if( numSongActs > 0 && 
                 currentSongAct >= numSongActs ) {
            // watch for act overflow on song change
            currentSongAct = numSongActs - 1;
            }
        
        

        printOut( "Available song parts:\n" );
        
        // look through all acts and get union of all possible part names
        SimpleVector<char *> allPartNames;
        
        for( int i=0; i<numSongActs; i++ ) {
            
            
            int numParts;
            
            char **songPartDirNames = listDirectory( songActDirNames[i], 
                                                     &numParts );
            

            if( numParts > 1 ) {
                sortStrings( &songPartDirNames, numParts );
                }
            
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
                
                if( numWavFiles > 1 ) {
                    sortStrings( &wavFileNames, numWavFiles );
                    }

                for( int w=0; w<numWavFiles; w++ ) {
                    char *wavName = wavFileNames[w];

                    int nameLength = (int)strlen( wavName );
                    
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
                            

                            if( stream.info.numSamples > gridStepLength ) {
                                // an even longer loop encountered
                                // use this as our grid step
            
                                gridStepLength = stream.info.numSamples;
                                printOut( 
                                    "New longer grid step discovered:  "
                                    "%d (%d sec)\n", 
                                    gridStepLength,
                                    gridStepLength / rate );
                                }
                            
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
        




        // now set up full file banks for each part/act
        songActFilesPerPart = new int*[ numSongActs ];
        songActPartFileBanks = new char***[ numSongActs ];
 
        for( int a=0; a<numSongActs; a++ ) {
            printOut( "Act %d:\n", a );
            
            songActFilesPerPart[a] = new int[ numSongParts ];
            songActPartFileBanks[a] = new char**[ numSongParts ];
            
            for( int p=0; p<numSongParts; p++ ) {
                printOut( "  Part %d:\n", p );
                
                songActPartFileBanks[a][p] = 
                    getAllFilesForActAndPart( 
                        a, p, &( songActFilesPerPart[a][p] ) );
                
                for( int f=0; f<songActFilesPerPart[a][p]; f++ ) {
                    printOut( "    Loop %d:  %s\n", f, 
                              songActPartFileBanks[a][p][f] );
                    }
                }
            }



        // between 2 and 4 minutes long, each
        currentSongTargetLength = rate * 60 * 2 + getRandom( rate * 60 * 2 );

        // we skip all but 2 seconds of our first grid step in the delay
        // between songs
        currentSongTargetLength += ( gridStepLength - rate * 2 );
        }
    else {
        printOut( "ERROR:  no songs present!\n" );

        currentSongPick = -1;
        }

    deleteArrayOfStrings( &songList, numSongs );
    }



// stops song and frees resources
static void unloadSong() {

    if( currentSongDirName != NULL ) {
        delete [] currentSongDirName;
        currentSongDirName = NULL;
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



    for( int a=0; a<numSongActs; a++ ) {
        
        
        for( int p=0; p<numSongParts; p++ ) {
            deleteArrayOfStrings( &( songActPartFileBanks[a][p] ),
                                  songActFilesPerPart[a][p] );
            }
        delete [] songActFilesPerPart[a];
        delete [] songActPartFileBanks[a];
        }
    delete [] songActFilesPerPart;
    delete [] songActPartFileBanks;

    songActFilesPerPart = NULL;
    songActPartFileBanks = NULL;


    }






// non-thread-safe version
// internal use only
static void setMusicStateInternal( const char *inStateString );








void initMusic() {

    
    

    // all channels start not playing
    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        s->wavBankStream = NULL;
        
        s->filePlaying = false;
        s->totalNumSamplesPlayed = 0;

        musicState[i] = -1;
        }


    // no music on clones
    // but at least init all channels to not playing above
    if( isThisAClone() ) {
        return;
        }

    loadSong();

    setMusicStateInternal( "START STATE" );

    }






void freeMusic() {

    // no music on clones
    if( isThisAClone() ) {
        return;
        }

    if( currentSongPick != -1 ) {
        unloadSong();
        }


    if( lastStateString != NULL ) {
        delete [] lastStateString;
        lastStateString = NULL;
        }
    }




static void addTrack( int inChannelNumber, int inDelay ) {
    
    int partPick = inChannelNumber;
    
    char *actDir = songActDirNames[ currentSongAct ];
    char *partName = songPartNames[ partPick ];

    int numPartFiles = songActFilesPerPart[currentSongAct][partPick];

    char **partFiles = songActPartFileBanks[currentSongAct][partPick];
        
            


    if( numPartFiles > 0 ) {
                            
        // wrap around in case musicState indexes a file greater than the 
        // max availabel (might happen after an act change if music state
        // hasn't been updated yet)
        int partFilePick = musicState[partPick] % numPartFiles;
        
        channelStream *s = &( songStreams[partPick] );

        // already rewound if it's not assigned to a channel
        s->wavBankStream = 
            findBankStreamByFileName( partFiles[ partFilePick ] );

        s->fileSamplePosition = 0;
        s->startSampleDelay = inDelay;
        s->filePlaying = true;
        
        printOut( "Adding loop %s on channel %d with delay %d\n", 
                  s->wavBankStream->wavFileName, partPick, inDelay );
        }
    else {
        printOut( "Part %s not found in song act %s (or in base act)\n",
                  partName, actDir );
        
        songStreams[partPick].filePlaying = false;
        }
    }



// use this as a safe way of gradually raising volume back up from zero,
// outside of audio callback
extern int globalSoundVolume;
extern char globalVolumeRise;

static char shouldStartVolumeRise = false;



void getAudioSamplesForChannel( int inChannelNumber, s16 *inBuffer, 
                                int inNumSamples ) {


    channelStream *s = &( songStreams[inChannelNumber] );

    unsigned int samplesLeftInGridStep = 
        gridStepLength - s->totalNumSamplesPlayed % gridStepLength;
    

    if( inChannelNumber == 0 ) {
        
        if( s->totalNumSamplesPlayed > currentSongTargetLength ) {
            // time to auto-switch

            // keep fading out
            globalVolumeRise = false;
            

            // but not until we've completed a nice fade-out
            if( globalSoundVolume == 0 ) {
                songSwitchPending = true;
                }
            // if not, we have to keep waiting
            }
        }
    




    // don't switch songs part-way through getting samples from channels
    if( songSwitchPending && inChannelNumber == 0 ) {


        
        




        // don't actually switch until they're all stopped
        char allStopped = true;
        
        for( int p=0; p<numSongParts; p++ ) {
            if( songStreams[p].filePlaying ) {
                allStopped = false;
                break;
                }
            }
            
        
        if( allStopped ) {
            
            // okay to switch songs
            printOut( "Song switch requested, and all tracks off and "
                      "at same position\n" );
            
            printOut( "Last song had %d acts\n", numSongActs );

            unloadSong();
            
            loadSong();
            

            // ensure a fixed amount of silence between songs (2 seconds)
            
            unsigned int sampleDelay = rate * 2;

            // reset totalNumSamplesPlayed for each track to set this up
            // position them each 2 seconds before next grid step
            for( int p=0; p<MAX_SOUND_CHANNELS; p++ ) {
                songStreams[p].totalNumSamplesPlayed = 
                    gridStepLength - sampleDelay;
                }
            
            printOut( "New song has %d acts\n", numSongActs );


            if( lastStateString != NULL ) {
                // redo music state to ensure it's consistent with new song
                setMusicStateInternal( lastStateString );
                }

            songSwitchPending = false;


            // jump immediately back to full volume for start of next
            // song
            // note that calling setSoundChannelVolume directly from inside 
            // the callback does not always work on the DS
            // but it works MOST of the time, so it's okay for lowering
            // the volume gradually above
            
            // But here we're trying to raise it all the way back up with a
            // single call.  If this call fails, then the volume stays at 0
            /*
            for( int c=0; c<MAX_SOUND_CHANNELS; c++ ) {
                setSoundChannelVolume( c, 127 );
                }
            */

            // so we let our game loop handle it, gradually raising it
            // note that we DID NOT set the globalSoundVolume above when
            // we lowered the volume.  The game loop only tries to adjust
            // volume up if it's not 127
            // so we left it at 127 when we turned volume down above
            // (so that game loop wouldn't touch volume)

            // globalSoundVolume = 0;

            // HOWEVER, wait until some loop is about to start
            // playing before triggering this (so we actually get a 
            // good fade-in, and don't start fading in during
            // the two seconds of intermediary silence)
            
            // Also, this ensures that we keep the volume down to cover
            // any glitches that might occur during the transition from
            // samples lingering in the audio buffer from the last loop
            // runs at the end of the fade out.
            // Holding the volume down during the 2 seconds of silence
            // ensures that these buffers get properly filled with silence
            // before the volume is raised, replacing any lingering
            // glitch audio in the buffer.
            shouldStartVolumeRise = true;
            }
        }

    




    int numSamplesRequested = inNumSamples;




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
            if( !songSwitchPending && 
                musicState[inChannelNumber] != -1 ) {
                
                addTrack( inChannelNumber, delay );

                if( shouldStartVolumeRise ) {
                    // this is the first track actually added after
                    // a song change.  Start the volume rise NOW.
                    globalVolumeRise = true;
                    
                    shouldStartVolumeRise = false;
                    }
                
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
                //printOut( "Reading from wav file for channel %d took %dms\n",
                //          inChannelNumber, netMS );
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

                

                // ONLY consider dropping out or changing on a FULL
                // grid step boundary
                // If we drop out part-way through a grid step, if we're the 
                // only loop playing during a musicState transition, we
                // may leave silence, because the next state's parts
                // will only come IN on a full grid step

                // otherwise, we simply rewind our wav and keep going
                if( s->totalNumSamplesPlayed / gridStepLength <
                    ( s->totalNumSamplesPlayed + numSamplesRequested ) 
                       / gridStepLength ) {
                    
                    
                    s->wavBankStream = NULL;

                    // consider dropping out
                    if( songSwitchPending || 
                        musicState[inChannelNumber] == -1 ) {
                        
                        printOut( "Track %d dropping out\n", inChannelNumber );
                        
                        s->filePlaying = false;
                        
                        // fill rest with silence
                        memset( inBuffer, 0, 
                                (unsigned int)( inNumSamples * 2 ) );
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
                                "Track %d dropping out because "
                                "of act change\n", 
                                inChannelNumber );
                        
                            // fill rest with silence
                            memset( inBuffer, 
                                    0, (unsigned int)( inNumSamples * 2 ) );
                            
                            inNumSamples = 0;
                            }
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
    
     
    return;

    }




void nextSongAct() {    
    if( isThisAClone() ) {
        return;
        }

    lockAudio();
    if( currentSongAct < numSongActs - 1 ) {
        currentSongAct ++;
        }

    if( lastStateString != NULL ) {
        // redo music state to ensure a new mix after act changes
        setMusicStateInternal( lastStateString );
        }
    
    

    unlockAudio();
    }



void backToFirstSongAct() {
    if( isThisAClone() ) {
        return;
        }

    lockAudio();
    currentSongAct = 0;
    
    if( numSongActs > 1 ) {
        currentSongAct = 1;
        }
    
    if( lastStateString != NULL ) {
        // redo music state to ensure a new mix after act changes
        setMusicStateInternal( lastStateString );
        }

    unlockAudio();
    }



int getSongAct() {    
    if( isThisAClone() ) {
        return 0;
        }


    int returnValue;
    
    lockAudio();
    returnValue = currentSongAct;
    unlockAudio();
    
    return returnValue;
    }



int getSongTimeLeft() {
    if( isThisAClone() ) {
        return 0;
        }
    
    int returnValue = 0;
    
    lockAudio();
    
    if( numSongParts > 0 ) {

        returnValue = 
            (int)currentSongTargetLength - 
            (int)songStreams[0].totalNumSamplesPlayed;

        if( songSwitchPending || returnValue < 0 ) {
            // show 0 to indicate that song is waiting to switch
            returnValue = 0;
            }
        }
    unlockAudio();
    
    return returnValue;
    }



char *getGridStepTimeString() {
    if( isThisAClone() ) {
        return NULL;
        }


    char *returnValue = NULL;
    

    lockAudio();
    
    if( numSongParts > 0 ) {

        unsigned int gridSamplesPlayed = 
            songStreams[0].totalNumSamplesPlayed % gridStepLength;
        
            
        returnValue = autoSprintf( "(%d.%d/%d.%d)",
                                   gridSamplesPlayed / rate, 
                                   gridSamplesPlayed / oneTenthRate - 
                                   10 * ( gridSamplesPlayed / rate ), 
                                   gridStepLength / rate,
                                   gridStepLength / oneTenthRate - 
                                   10 * ( gridStepLength / rate ) );
        }
    unlockAudio();
    
    return returnValue;
    }






void switchSongs() {    
    if( isThisAClone() ) {
        return;
        }

    lockAudio();

    songSwitchPending = true;

    unlockAudio();
    }




char **getTrackInfoStrings( int *outNumTracks ) {
    lockAudio();
    
    char **strings = new char*[MAX_SOUND_CHANNELS];
    
    for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
        
        channelStream *s = &( songStreams[i] );

        if( s->filePlaying ) {
            strings[i] = autoSprintf( 
                "%d:  %s (%d.%d/%d.%d)", i, s->wavBankStream->wavFileName,
                s->fileSamplePosition / rate, 
                s->fileSamplePosition / oneTenthRate - 
                  10 * ( s->fileSamplePosition / rate ), 
                s->wavBankStream->info.numSamples / rate,
                s->wavBankStream->info.numSamples / oneTenthRate - 
                  10 * ( s->wavBankStream->info.numSamples / rate ) );
            }
        else {
            strings[i] = autoSprintf( "%d:  OFF", i );
            }
        
        }
    
    *outNumTracks = MAX_SOUND_CHANNELS;
    
    unlockAudio();
    
    return strings;
    }






void setMusicState( const char *inStateString ) {

    if( isThisAClone() ) {
        return;
        }
    
    lockAudio();
    
    setMusicStateInternal( inStateString );
    
    unlockAudio();
    }



void setMusicStateInternal( const char *inStateString ) {
    

    // convert to int hash so we can use it to seed a deterministic
    // random process

    // add our act number into it so that different acts with the same
    // state string produce different mixes
    char *fullStringToHash = autoSprintf( "%s%d", inStateString, 
                                          currentSongAct );
    
    unsigned int hash = crcHash( (unsigned char*)fullStringToHash, 
                                 strlen( fullStringToHash ) );
    
    delete [] fullStringToHash;


    
    char *oldLastStateString = lastStateString;
    
    lastStateString = stringDuplicate( inStateString );
    
    if( oldLastStateString != NULL ) {
        delete [] oldLastStateString;
        }
    
    


    randState state = startCustomRand( hash );
    
    randState *s = &state;
    


    int numPlaying = 0;
    
    for( int p=0; p<numSongParts; p++ ) {
        // turn all off to start
        musicState[p] = -1;
        }
    


    // keep adding parts until at least one is playing

    // and after that, if ONLY one is playing, add one more

    // also might force all 5 tracks on, for testing.
    while( ( !forceFiveTracks && numPlaying < 2 ) 
           ||
           ( forceFiveTracks && numPlaying < 5 ) ) {

        char justAddOneMore = false;
        
        if( numPlaying == 1 && !forceFiveTracks ) {
            // we've already been through our complete loop, where each track
            // had a fair-coin chance to play, exactly once
            
            // but on rare occasions, we only enable one track that way
            // and one track is too sparse sounding

            // SO, add one more in that case.  Never have just one by itself

            // if we simply run through our entire loop a second time
            // and flip again for all off tracks again, it ends up turning too
            // many on, on average
            justAddOneMore = true;
            }
        
        
        // go through all parts and flip a fair coin to decide if it is playing
        // thus, on average, half of our tracks are playing
        
        // rarest to have ALL playing.
        // rarest to have ONE playing.
        
        for( int p=0; p<numSongParts; p++ ) {
    
            if( // currently off
                musicState[p] == -1 &&
                // win a fair coin flip
                getRandom( s, 100 ) > 50 ) {
                
                int numFilesThisAct = songActFilesPerPart[currentSongAct][p];
                
                if( numFilesThisAct > 0 ) {
                    // pick one
                    musicState[p] = 
                        (int)getRandom( s, (unsigned int)numFilesThisAct );

                    numPlaying ++;

                    if( justAddOneMore ) {
                        break;
                        }
                    }
                }
            }
        }

    if( limitTotalTracks && numPlaying > 2 ) {
        // now go back through and trim out tracks if we have too many
    
        while( numPlaying > 2 ) {
            
            for( int p=0; p<numSongParts && numPlaying > 2; p++ ) {
                
                if( // currently on
                    musicState[p] != -1 &&
                    // win a fair coin flip
                    getRandom( s, 100 ) > 50 ) {
                    
                    
                    // turn off
                    musicState[p] = -1;
                    numPlaying--;
                    }
                }
            }
        }
    


    }




char *getLastMusicState() {
    
    char *returnValue = NULL;
    
    lockAudio();
    
    if( lastStateString != NULL ) {    
        returnValue = stringDuplicate( lastStateString );
        }
    
    unlockAudio();
    
    return returnValue;
    }



void limitTotalMusicTracks( char inLimit ) {
    // ignore all of this on a clone (no music)
    if( isThisAClone() ) {
        return;
        }

    limitTotalTracks = inLimit;

    // redo music state to ensure it's consistent with new limit
    setMusicStateInternal( lastStateString );
    }





