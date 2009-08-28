#ifndef PLATFORM_INCLUDED
#define PLATFORM_INCLUDED

// interface for a game platform
// separates platform-dependent code from platform independent code
// platform-dependent code provides the main loop


// **
// functions that must be implemented by the platform
// **


void *allocMem( unsigned int inSizeInBytes );
void  freeMem( void *inRegion );

// reads full contents of a file into newly-allocated memory
unsigned char *readFile( char *inFileName, int *outSize );


// conforms to specification for printf, generally
void printOut( const char *inFormatString, ... );



struct rgbaColorStruct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
typedef struct rgbaColorStruct rgbaColor;


// creates sprite from 32-bit image data
// returns handle to sprite
int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight );

// draws sprite at position and with color, including alpha for transparency
// sprite is drawn on the currently-drawing screen
void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor );




// **
// functions that must be implemented by the platform-independent code
// **

// inits the game
// called by platform before entering main loop 
void gameInit();

// called during each iteration of main loop
void gameLoopTick();

// called when drawing is initiated for each screen
// all drawSprite must occur in these functions.
void drawTopScreen();
void drawBottomScreen();


#endif


