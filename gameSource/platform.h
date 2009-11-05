#ifndef PLATFORM_INCLUDED
#define PLATFORM_INCLUDED

// interface for a game platform
// separates platform-dependent code from platform independent code
// platform-dependent code provides the main loop


// **
// functions that must be implemented by the platform
// **


/*
void *allocMem( unsigned int inSizeInBytes );
void freeMem( void *inRegion );

// matches signature of stdc memcpy
void copyMem( void *inDest, void *inSource, unsigned int inSizeInBytes );
*/

// some platforms must override new and delete
#if defined( SDK_TWL ) || defined( SDK_NITRO )
  void *operator new( std::size_t inSizeInBytes );
  void *operator new[] ( std::size_t inSizeInBytes );
  void operator delete( void *inRegion ) throw();
  void operator delete[] ( void *inRegion )throw();
#endif



// reads full contents of a file into newly-allocated memory
unsigned char *readFile( char *inFileName, int *outSize );


// conforms to specification for printf, generally
void printOut( const char *inFormatString, ... );


// random value greater than or equal to 0 and less than inMax
unsigned int getRandom( unsigned int inMax );



typedef struct rgbaColor {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } rgbaColor;


// creates sprite from 32-bit image data
// returns handle to sprite
// data destroyed by caller
int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight );



// add a 256-color sprite (1 byte per pixel) that indexes into a 16-bit
// palette (ABGR 1555)
int addSprite256( unsigned char *inDataBytes, int inWidth, int inHeight,
                  unsigned short inPalette[256] );


// replaces sprite with new image data
//
// Only works on 256-color images
//
// inSpriteID must be an ID previously returned by addSprite
// new image must have same dimensions as original
// original palette is also reused
void replaceSprite256( int inSpriteID, 
                       unsigned char *inDataRGBA, int inWidth, int inHeight );




// draws sprite at position and with color, including alpha for transparency
// sprite is drawn on the currently-drawing screen
void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor );

// puts the next batch of sprites on top of the previous batches
// within a batch, between calls to startNewSpriteLayer, sprites should
// not overlap (to avoid sorting artifacts)
// If two sprites overlap, the top one should be drawn during a later layer
void startNewSpriteLayer();


// gets the current position of a pressed mouse or stylus
// returns true if pressed
char getTouch( int *outX, int *outY );




// true if platform is auto-connecting to a server at startup
// no need to acceptConnection or connectToServer
// check if connection ready with checkConnectionReady
char isAutoconnecting();


// local address (where clients should connect) as a newly allocated string
// NULL on address-free platforms (like DS)
char *getLocalAddress();

// start a server that waits for a connection
void acceptConnection();

// start a client connection process
// inAddress can be NULL on address-free platforms
void connectToServer( char *inAddress );


// 1 if a connection has been established and is still up, 
// 0 if still connecting, 
// -1 on error
int checkConnectionStatus();


// inMessage destroyed by caller
void sendMessage( unsigned char *inMessage, unsigned int inLength );


// poll for new incoming message
// returns NULL if no message ready
// result freed by caller
unsigned char *getMessage( unsigned int *outLength );



// camera support

// true if this platform has a camera
// NOTE:  on DS, try  BOOL OS_IsRunOnTwl( void );
char isCameraSupported();

// start producing frames
void startCamera();

// stop producing frames
void stopCamera();

// Trimming size fixed at 160x120
// format fixed at grayscale 256 levels

// get the next frame
// inBuffer is where 160x120 grayscale pixels will be returned
void getFrame( unsigned char *inBuffer );

// snap the next frame as a finished picture
void snapPicture( unsigned char *inBuffer );







// **
// functions that must be implemented by the platform-independent code
// **

// inits the game
// called by platform before entering main loop 
void gameInit();

// frees the game
// called when app exits
void gameFree();


// called during each iteration of main loop
void gameLoopTick();

// called when drawing is initiated for each screen
// all drawSprite must occur in these functions.
void drawTopScreen();
void drawBottomScreen();


#endif


