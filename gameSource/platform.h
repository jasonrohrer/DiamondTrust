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
void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor );




// **
// functions that must be implemented by the platform-independent code
// **

// inits the game
// called by platform before entering main loop 
void gameInit();

// called during each iteration of main loop
void gameLoopTick();



