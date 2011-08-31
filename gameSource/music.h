

void initMusic();


void freeMusic();



// pass in a string that describes target music state (like state of game)
// this string is mapped to a particular arrangement of music loops, which 
// will go into effect at the next transition point.

// passing in the same string will always return the music arrangement to the
// same state in a given act
void setMusicState( const char *inStateString );


// destroyed by caller
// can be NULL
char *getLastMusicState();


// put music into limited mode, where fewer max tracks can play at once
// (do during cpu-cricital times during game to prevent music from skipping)
void limitTotalMusicTracks( char inLimit );

char getMusicTrackLimitOn();




// move on to the next act in the song
void nextSongAct();

void backToFirstSongAct();


int getSongAct();

void forceSongAct( int inAct );

// when enabled, a random act is picked at every song transition
// otherwise, song acts advance according to nextSongAct calls
void enableActChangesWithEverySong( char inActChangesWithEverySong );




// time left in samples
int getSongTimeLeft();

char *getGridStepTimeString();



// set inNexSong to 1 to advance song by exactly 1
// set to -1 to go back by one song
// 0 (default) picks a random song
// calling this multiple times, before the actual song transition, 
// is cumulative (can jump by more than one song)
void switchSongs( int inNexSong=0 );



// resulting array destroyed by caller
char **getTrackInfoStrings( int *outNumTracks );
