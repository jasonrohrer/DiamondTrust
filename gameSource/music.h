

void initMusic();


void freeMusic();



// pass in a string that describes target music state (like state of game)
// this string is mapped to a particular arrangement of music loops, which 
// will go into effect at the next transition point.

// passing in the same string will always return the music arrangement to the
// same state in a given act
void setMusicState( char *inStateString );


// NOT destroyed by caller
// can be NULL
char *getLastMusicState();



// move on to the next act in the song
void nextSongAct();


int getSongAct();


// resulting array destroyed by caller
char **getTrackInfoStrings( int *outNumTracks );
