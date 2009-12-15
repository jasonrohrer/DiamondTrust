// SDL platform that runs game.h engine


// let SDL override our main function with SDLMain
#include <SDL/SDL_main.h>


// must do this before SDL include to prevent WinMain linker errors on win32
int mainFunction( int inArgCount, char **inArgs );

int main( int inArgCount, char **inArgs ) {
    return mainFunction( inArgCount, inArgs );
    }


#include <SDL/SDL.h>



#include "platform.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <GL/gl.h>


#include "minorGems/util/stringUtils.h"


// callbacks, originally for GLUT, called by our SDL main loop below
void callbackMotion( int inX, int inY );
// ignore passive motion
// void callbackPassiveMotion( int inX, int inY );
void callbackMouse( int inButton, int inState, int inX, int inY );
void callbackDisplay();
void callbackTimer( int inValue );

// keyboard to catch quit keys (don't pass to game)
void callbackKeyboard( unsigned char inKey, int inX, int inY );


void callbackReshape( int inW, int inH );


void stepNetwork();




// maps SDL-specific special (non-ASCII) key-codes (SDLK) to minorGems key 
// codes (MG_KEY)
//int mapSDLSpecialKeyToMG( int inSDLKey );

// for ascii key
char mapSDLKeyToASCII( int inSDLKey );





void freeSprites();
void freeNetwork();


void cleanUpAtExit() {
    printf( "Exiting...\n" );

    //SDL_CloseAudio();
    
    gameFree();    
    freeSprites();
    freeNetwork();
    }



void audioCallback( void *inUserData, Uint8 *inStream, int inLengthToFill ) {
    //getSoundSamples( inStream, inLengthToFill );
    }



int w = 256;
int h = 384;

int blowupFactor = 1;

int bottomScreenYOffset = 192;
int singleScreenH = 192;


// 30 fps
int frameMS = 33;
//int frameMS = 500;

// s and f keys to slow down and speed up for testing
char enableSlowdownKeys = false;
//char enableSlowdownKeys = true;


char touchWasDown = false;
char touchDown = false;
int touchX = 0;
int touchY = 0;




int mainFunction( int inNumArgs, char **inArgs ) {


    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) < 0 ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        return 0;
        }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
    int flags = SDL_OPENGL;
    

    //flags = flags | SDL_FULLSCREEN;
    
    // current color depth
    SDL_Surface *screen = SDL_SetVideoMode( w * blowupFactor, 
                                            h * blowupFactor, 0, flags);


    if ( screen == NULL ) {
        printf( "Couldn't set %dx%d GL video mode: %s\n", 
                w * blowupFactor, 
                h * blowupFactor,
                SDL_GetError() );
        return 0;
        }
    

    SDL_WM_SetCaption( "gameApp", NULL );
    

    // turn off repeat
    SDL_EnableKeyRepeat( 0, 0 );



    
    #ifdef __mac__
        // make sure working directory is the same as the directory
        // that the app resides in
        // this is especially important on the mac platform, which
        // doesn't set a proper working directory for double-clicked
        // app bundles

        // arg 0 is the path to the app executable
        char *appDirectoryPath = stringDuplicate( inArgs[0] );
    
        char *appNamePointer = strstr( appDirectoryPath,
                                       "gameApp.app" );

        if( appNamePointer != NULL ) {
            // terminate full app path to get parent directory
            appNamePointer[0] = '\0';
            
            chdir( appDirectoryPath );
            }
        
        delete [] appDirectoryPath;
    #endif



    



    SDL_AudioSpec audioFormat;

    /* Set 16-bit stereo audio at 22Khz */
    //audioFormat.freq = gameSoundSampleRate;
    audioFormat.format = AUDIO_S16;
    audioFormat.channels = 2;
    audioFormat.samples = 512;        /* A good value for games */
    //audioFormat.samples = 1024;        
    //audioFormat.samples = 2048;        
    audioFormat.callback = audioCallback;
    audioFormat.userdata = NULL;

    /* Open the audio device and start playing sound! */
    /*
      // Audio disabled for now
    if( SDL_OpenAudio( &audioFormat, NULL ) < 0 ) {
        printf( "Unable to open audio: %s\n", SDL_GetError() );
        }
    */


    // set pause to 0 to start audio
    //SDL_PauseAudio(0);


    gameInit();
    
    // to free frame drawer, stop audio, etc
    atexit( cleanUpAtExit );


    Uint32 lastFrameDoneTime = SDL_GetTicks();

    Uint32 frameBatchStartTime = SDL_GetTicks();
    
    int frameBatchSize = 100;
    int frameBatchCount = 0;
    
    

    
    // main loop
    while( true ) {
        
        stepNetwork();
        
        gameLoopTick();
        
        callbackDisplay();
        

        // handle all pending events
        SDL_Event event;
        
        while( SDL_PollEvent( &event ) ) {
            
            switch( event.type ) {
                case SDL_QUIT:
                    exit( 0 );
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    int mouseX, mouseY;
                    SDL_GetMouseState( &mouseX, &mouseY );
                    
                    
                    // check if special key
                    //int mgKey = mapSDLSpecialKeyToMG( event.key.keysym.sym );
                    // ignore special keys for now
                    int mgKey = 0;
                    
                    
                    if( mgKey != 0 ) {
                        if( event.type == SDL_KEYDOWN ) {
                            //callbackSpecialKeyboard( mgKey, mouseX, mouseY );
                            }
                        else {
                            //callbackSpecialKeyboardUp( mgKey, 
                            // mouseX, mouseY );
                            }
                        }
                    else {
                        char asciiKey = 
                            mapSDLKeyToASCII( event.key.keysym.sym );
                      
                        if( asciiKey != 0 ) {
                            // shift and caps cancel each other
                            if( ( ( event.key.keysym.mod & KMOD_SHIFT )
                                  &&
                                  !( event.key.keysym.mod & KMOD_CAPS ) )
                                ||
                                ( !( event.key.keysym.mod & KMOD_SHIFT )
                                  &&
                                  ( event.key.keysym.mod & KMOD_CAPS ) ) ) {
                                
                                asciiKey = toupper( asciiKey );
                                }
                        
                            if( event.type == SDL_KEYDOWN ) {
                                callbackKeyboard( asciiKey, mouseX, mouseY );
                                }
                            else {
                                //callbackKeyboardUp( asciiKey, 
                                // mouseX, mouseY );
                                }
                            }
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if( event.motion.state & SDL_BUTTON( 1 )
                        || 
                        event.motion.state & SDL_BUTTON( 2 )
                        ||
                        event.motion.state & SDL_BUTTON( 3 ) ) {
                        
                        callbackMotion( event.motion.x, event.motion.y );
                        }
                    else {
                        //callbackPassiveMotion( event.motion.x, 
                        // event.motion.y );
                        }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    callbackMouse( event.button.button,
                                   event.button.state,
                                   event.button.x, 
                                   event.button.y );
                    break;
                }
            }

        Uint32 thisFrameDoneTime = SDL_GetTicks();
        
        int extraMS = frameMS - ( thisFrameDoneTime - lastFrameDoneTime );
        
        if( extraMS > 0 ) {
            SDL_Delay( extraMS );
            // set to ideal frame done time (even though SDL_Delay might
            // be slightly longer or shorter
            
            // this way, we don't fall gradually out of time
            lastFrameDoneTime = thisFrameDoneTime + extraMS;
            }
        else {
            // frame took too long
            // allow this slow-down now so we don't have a jarring speedup
            // to catch up later
            lastFrameDoneTime = thisFrameDoneTime;
            }
        




        frameBatchCount ++;
        
        if( frameBatchCount >= frameBatchSize ) {
            Uint32 doneTime = SDL_GetTicks();
            Uint32 totalTime =  doneTime - frameBatchStartTime;
            
            float fps = (float)frameBatchCount / (totalTime / 1000.0f);
            
            if( false ) {
                printf( "FPS = %f\n", fps );
                }
            


            // new batch
            frameBatchStartTime = doneTime;
            frameBatchCount = 0;
            }
        
        }


    return 0;    
    }



void setSoundPlaying( char inPlaying ) {
    printf( "Setting audio playing to %d\n", inPlaying );
    
    SDL_PauseAudio( !inPlaying );
    }



int spriteYOffset = 0;


void callbackDisplay() {
		
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, w, singleScreenH, 0, -1.0f, 1.0f);
    
    
    glMatrixMode(GL_MODELVIEW);

    glViewport( 0, blowupFactor * (h - singleScreenH), 
                blowupFactor * w, blowupFactor * singleScreenH );
    
    
    spriteYOffset = 0;
    drawTopScreen();
    
    
	//spriteYOffset = bottomScreenYOffset;
    glViewport( 0, 0, 
                blowupFactor * w, blowupFactor * singleScreenH );
    drawBottomScreen();


    // separating line:
    glColor4ub( 255, 255, 255, 255 );
    const GLfloat lineVertices[] = {
        0, 0,
        w * blowupFactor, 0 
        };
    
    glVertexPointer( 2, GL_FLOAT, 0, lineVertices );
    glEnableClientState( GL_VERTEX_ARRAY );
    
        
    glDrawArrays( GL_LINES, 0, 2 );

    SDL_GL_SwapBuffers();
	}




void callbackMotion( int inX, int inY ) {
    if( inY >= bottomScreenYOffset ) {
        
        touchX = inX;
        touchY = inY - bottomScreenYOffset;
        touchDown = true;
        touchWasDown = true;
        }
    else {
        touchDown = false;
        } 
	}
		
	
	
void callbackMouse( int inButton, int inState, int inX, int inY ) {
	if( inState == SDL_PRESSED ) {
        callbackMotion( inX, inY );
        }
    else if( inState == SDL_RELEASED ) {
        touchDown = false;
        }
    else {
        printf( "Error:  Unknown mouse state received from SDL\n" );
        }	
    }


void callbackKeyboard( unsigned char inKey, int inX, int inY ) {
    
    // q or escape
    if( inKey == 'q' || inKey == 'Q' || inKey == 27 ) {
        exit( 0 );
        }
    

    if( enableSlowdownKeys ) {
        
        if( inKey == 's' || inKey == 'S' ) {
            // slow
            frameMS = 500;
            }
        if( inKey == 'f' || inKey == 'F' ) {
            // normal
            frameMS = 40;
            }
        }
    
    
	}


/*
int mapSDLSpecialKeyToMG( int inSDLKey ) {
    switch( inSDLKey ) {
        case SDLK_F1: return MG_KEY_F1;
        case SDLK_F2: return MG_KEY_F2;
        case SDLK_F3: return MG_KEY_F3;
        case SDLK_F4: return MG_KEY_F4;
        case SDLK_F5: return MG_KEY_F5;
        case SDLK_F6: return MG_KEY_F6;
        case SDLK_F7: return MG_KEY_F7;
        case SDLK_F8: return MG_KEY_F8;
        case SDLK_F9: return MG_KEY_F9;
        case SDLK_F10: return MG_KEY_F10;
        case SDLK_F11: return MG_KEY_F11;
        case SDLK_F12: return MG_KEY_F12;
        case SDLK_LEFT: return MG_KEY_LEFT;
        case SDLK_UP: return MG_KEY_UP;
        case SDLK_RIGHT: return MG_KEY_RIGHT;
        case SDLK_DOWN: return MG_KEY_DOWN;
        case SDLK_PAGEUP: return MG_KEY_PAGE_UP;
        case SDLK_PAGEDOWN: return MG_KEY_PAGE_DOWN;
        case SDLK_HOME: return MG_KEY_HOME;
        case SDLK_END: return MG_KEY_END;
        case SDLK_INSERT: return MG_KEY_INSERT;
        default: return 0;
        }
    }
*/

char mapSDLKeyToASCII( int inSDLKey ) {
    switch( inSDLKey ) {
        case SDLK_UNKNOWN: return 0;
        case SDLK_BACKSPACE: return 8;
        case SDLK_TAB: return 9;
        case SDLK_CLEAR: return 12;
        case SDLK_RETURN: return 13;
        case SDLK_PAUSE: return 19;
        case SDLK_ESCAPE: return 27;
        case SDLK_SPACE: return ' ';
        case SDLK_EXCLAIM: return '!';
        case SDLK_QUOTEDBL: return '"';
        case SDLK_HASH: return '#';
        case SDLK_DOLLAR: return '$';
        case SDLK_AMPERSAND: return '&';
        case SDLK_QUOTE: return '\'';
        case SDLK_LEFTPAREN: return '(';
        case SDLK_RIGHTPAREN: return ')';
        case SDLK_ASTERISK: return '*';
        case SDLK_PLUS: return '+';
        case SDLK_COMMA: return ',';
        case SDLK_MINUS: return '-';
        case SDLK_PERIOD: return '.';
        case SDLK_SLASH: return '/';
        case SDLK_0: return '0';
        case SDLK_1: return '1';
        case SDLK_2: return '2';
        case SDLK_3: return '3';
        case SDLK_4: return '4';
        case SDLK_5: return '5';
        case SDLK_6: return '6';
        case SDLK_7: return '7';
        case SDLK_8: return '8';
        case SDLK_9: return '9';
        case SDLK_COLON: return ':';
        case SDLK_SEMICOLON: return ';';
        case SDLK_LESS: return '<';
        case SDLK_EQUALS: return '=';
        case SDLK_GREATER: return '>';
        case SDLK_QUESTION: return '?';
        case SDLK_AT: return '@';
        case SDLK_LEFTBRACKET: return '[';
        case SDLK_BACKSLASH: return '\\';
        case SDLK_RIGHTBRACKET: return ']';
        case SDLK_CARET: return '^';
        case SDLK_UNDERSCORE: return '_';
        case SDLK_BACKQUOTE: return '`';
        case SDLK_a: return 'a';
        case SDLK_b: return 'b';
        case SDLK_c: return 'c';
        case SDLK_d: return 'd';
        case SDLK_e: return 'e';
        case SDLK_f: return 'f';
        case SDLK_g: return 'g';
        case SDLK_h: return 'h';
        case SDLK_i: return 'i';
        case SDLK_j: return 'j';
        case SDLK_k: return 'k';
        case SDLK_l: return 'l';
        case SDLK_m: return 'm';
        case SDLK_n: return 'n';
        case SDLK_o: return 'o';
        case SDLK_p: return 'p';
        case SDLK_q: return 'q';
        case SDLK_r: return 'r';
        case SDLK_s: return 's';
        case SDLK_t: return 't';
        case SDLK_u: return 'u';
        case SDLK_v: return 'v';
        case SDLK_w: return 'w';
        case SDLK_x: return 'x';
        case SDLK_y: return 'y';
        case SDLK_z: return 'z';
        case SDLK_DELETE: return 127;
        default: return 0;
        }
    }

    





// implement the platform.h stuff here:


#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"

unsigned char *readFile( char *inFileName, int *outSize ) {
    // files are inside gameData directory


    File f( new Path( "gameData" ), inFileName );
    
    if( ! f.exists() ) {
        return NULL;
        }
    
    *outSize = f.getLength();
    
    return (unsigned char*) f.readFileContents();
    }



#include <stdarg.h>

void printOut( const char *inFormatString, ... ) {
    va_list argList;
    va_start( argList, inFormatString );
    
    vprintf( inFormatString, argList );
    
    va_end( argList );
    }




#include <time.h>

unsigned int getSecondsSinceEpoc() {
    return time( NULL );
    }




#include "minorGems/util/SimpleVector.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"


typedef struct texturePalette {
        rgbaColor fullColorPalette[256];
    } texturePalette;



SimpleVector<SingleTextureGL*> spriteTextures;
SimpleVector<unsigned int> spriteWidths;
SimpleVector<unsigned int> spriteHeights;
// save palettes to be reused if 8-bit texture data changes
// these will only contain placeholders for non-8-bit sprites
SimpleVector<texturePalette> spritePalettes;


void freeSprites() {
    for( int i=0; i<spriteTextures.size(); i++ ) {
        SingleTextureGL *t = *( spriteTextures.getElement( i ) );
        delete t;
        }
    }


static int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight,
                      texturePalette *inP ) {
    
    SingleTextureGL *s = new SingleTextureGL( (unsigned char*)inDataRGBA,
                                              inWidth, inHeight );
    spriteTextures.push_back( s );
    
    spriteWidths.push_back( inWidth );
    spriteHeights.push_back( inHeight );
    
    spritePalettes.push_back( *inP );

    return spriteTextures.size() - 1;
    }



int createSpriteSet() {
    return -1;
    }


void makeSpriteActive( int inHandle, char inReplaceSafe ) {
    // ignore sprite sets on SDL
    }

char isSpriteReady( int inHandle ) {
    // ignore sprite sets on SDL
    return true;
    }




int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight, 
               int inSetID ) {
    // ignore sprite sets on SDL

    // placeholder
    texturePalette p;

    return addSprite( inDataRGBA, inWidth, inHeight, &p );
    }


int addSprite256( unsigned char *inDataBytes, int inWidth, int inHeight,
                  unsigned short inPalette[256], char inZeroTransparent,
                  int inSetID ) {
    // ignore sprite sets on SDL
    
    // map palette to RGBA colors
    texturePalette p;
    
    for( int c=0; c<256; c++ ) {
        unsigned short colorShort = inPalette[c];
        
        p.fullColorPalette[c].r = ( colorShort & 0x001F ) << 3;
        p.fullColorPalette[c].g = ( (colorShort >>  5) & 0x001F ) << 3;
        p.fullColorPalette[c].b = ( (colorShort >> 10) & 0x001F ) << 3;
        // force non-transparent for all
        p.fullColorPalette[c].a = 255;    
        }

    if( inZeroTransparent ) {
        // color zero in palette represents transparent areas
        p.fullColorPalette[0].a = 0;
        }
    
    
    int numPixels = inWidth * inHeight;
    rgbaColor *fullColorImage = new rgbaColor[ numPixels ];
    for( int i=0; i<numPixels; i++ ) {
        fullColorImage[i] = p.fullColorPalette[ inDataBytes[i] ];
        }
    
    int id = addSprite( fullColorImage, inWidth, inHeight, &p );
    
    delete [] fullColorImage;
    
    return id;
    }


void replaceSprite256( int inSpriteID, 
                       unsigned char *inDataBytes, 
                       int inWidth, int inHeight, char inReplaceSafe ) {
    
    // no need for safe replacement on SDL

    texturePalette p = *( spritePalettes.getElement( inSpriteID ) );

    int numPixels = inWidth * inHeight;
    
    rgbaColor *fullColorImage = new rgbaColor[ numPixels ];
    for( int i=0; i<numPixels; i++ ) {
        fullColorImage[i] = p.fullColorPalette[ inDataBytes[i] ];
        }
    
    SingleTextureGL *s = *( spriteTextures.getElement( inSpriteID ) );

    s->replaceTextureData( (unsigned char*)fullColorImage,
                           inWidth, inHeight );

    delete [] fullColorImage;
    }




void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor ) {
    
    // account for "screen" being drawn on
    inY += spriteYOffset;
    

    glColor4ub( inColor.r, inColor.g, inColor.b, inColor.a );
        
    SingleTextureGL *texture = *( spriteTextures.getElement( inHandle ) );
    

    texture->enable(); 
    unsigned int spriteW = *( spriteWidths.getElement( inHandle ) );
    unsigned int spriteH = *( spriteHeights.getElement( inHandle ) );
    

    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );


    const GLfloat squareVertices[] = {
        inX, inY,
        inX + spriteW, inY,
        inX, inY + spriteH,
        inX + spriteW, inY + spriteH,
        };


    const GLfloat squareTextureCoords[] = {
        0, 0,
        1, 0,
        0, 1,
        1, 1
        };

    

    glVertexPointer( 2, GL_FLOAT, 0, squareVertices );
    glEnableClientState( GL_VERTEX_ARRAY );
    
    
    glTexCoordPointer( 2, GL_FLOAT, 0, squareTextureCoords );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /*

    glBegin( GL_QUADS ); {
    
        glTexCoord2f( 0, inSubsectionOffset );
        glVertex2f( inCenterX - inXRadius, inCenterY - inYRadius );
        
        glTexCoord2f( 1, inSubsectionOffset );
        glVertex2f( inCenterX + inXRadius, inCenterY - inYRadius );
        
        glTexCoord2f( 1, inSubsectionOffset + inSubsectionExtent );
        glVertex2f( inCenterX + inXRadius, inCenterY + inYRadius );
        
        glTexCoord2f( 0, inSubsectionOffset + inSubsectionExtent );
        glVertex2f( inCenterX - inXRadius, inCenterY + inYRadius );        
        }
    glEnd();
    */
    texture->disable();
    }


void startNewSpriteLayer() {
    // no layering necessary in SDL version
    }



// gets the current position of a pressed mouse or stylus
// returns true if pressed
// also consume last touch right before release, even if not currently
// pressed, to avoid missing events between checks
char getTouch( int *outX, int *outY ) {
    if( touchDown || touchWasDown ) {
        *outX = touchX;
        *outY = touchY;
        
        if( ! touchDown ) {
            // consume it
            touchWasDown = false;
            }
        

        return true;
        }
    
    return false;
    }





int netStatus = 0;



char isAutoconnecting() {
    return false;
    }



#include "minorGems/network/HostAddress.h"


char *getLocalAddress() {
    HostAddress *localAddr = HostAddress::getLocalAddress();
    char *returnValue = stringDuplicate( localAddr->mAddressString );
    
    delete localAddr;
    
    return returnValue;
    }


#include "minorGems/network/SocketServer.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/Socket.h"

Socket *connectionSock = NULL;

SocketServer *server = NULL;

int portNumber = 8641;



void acceptConnection() {
    netStatus = 0;
    server = new SocketServer( portNumber, 5 );
    }



void connectToServer( char *inAddress ) {
    
    // ignore address for now

    // read address from file
    File f( new Path( "gameData" ), "serverAddress.ini" );
    
    if( ! f.exists() ) {
        printOut( "Failed to read serverAddress.ini\n" );
        
        netStatus = -1;
        return;
        }
        
    char *address = f.readFileContents();
    
    if( address == NULL ) {
        printOut( "Failed to read serverAddress.ini\n" );
        
        netStatus = -1;
        return;
        }
    

    printOut( "Connecting to %s\n", address );
    
    HostAddress h( address, portNumber );
    
    char timedOut;
    // non-blocking
    // check socket later for connection status
    connectionSock = SocketClient::connectToServer( &h,
                                                    0,
                                                    &timedOut );

    if( connectionSock == NULL ) {
        netStatus = -1;
        }
    else {
        netStatus = 0;
        }    
    }



int checkConnectionStatus() {
    return netStatus;
    }


#include "DataFifo.h"

DataFifo sendFifo;
DataFifo receiveFifo;

unsigned int nextIncomingMessageSize = 0;
unsigned char *nextIncomingMessage = NULL;
unsigned int nextIncomingBytesRecievedSoFar = 0;



void sendMessage( unsigned char *inMessage, unsigned int inLength ) {
    sendFifo.addData( inMessage, inLength );
    }


unsigned char *getMessage( unsigned int *outLength ) {
    return receiveFifo.getData( outLength );
    }



void stepNetwork() {
    if( netStatus == 0 ) {
        
        if( server != NULL ) {
        
            char timedOut;
            
            connectionSock = server->acceptConnection( 0, &timedOut );
            
            if( connectionSock != NULL ) {
                printOut( "Got connection\n" );
                
                netStatus = 1;
                }
            else if( !timedOut ) {
                // error
                netStatus = -1;
                printOut( "Incoming connection error\n");
                }
            }        
        else if( connectionSock != NULL ) {
            if( netStatus == 0 ) {
                // check if connected
                netStatus = connectionSock->isConnected();
                
                if( netStatus == -1 ) {
                    printOut( "Error connecting to server\n");
                    }
                }
            }
        }
    else if( netStatus == 1 ) {

        // try sending next message
        unsigned int numBytes;
            
        unsigned char *nextData = sendFifo.peekData( &numBytes );
            
        if( nextData != NULL ) {
            // first 4 bytes are size
            unsigned int sendSize = numBytes + 4;
            unsigned char *sendData = new unsigned char[ sendSize ];
                
            sendData[0] = ( numBytes >> 24 ) & 0xFF;
            sendData[1] = ( numBytes >> 16 ) & 0xFF;
            sendData[2] = ( numBytes >> 8 ) & 0xFF;
            sendData[3] = ( numBytes ) & 0xFF;
                
            memcpy( &( sendData[4] ), nextData, numBytes );
                
            delete [] nextData;
                
            // non-blocking
            int numSent = 
                connectionSock->send( sendData, sendSize, false );

            
            delete [] sendData;
                
            if( numSent == -1 ) {
                // error
                netStatus = -1;
                printOut( "Send error\n");
                return;
                }
            else if( numSent == (int)sendSize ) {
                // done
                    
                // remove from fifo
                nextData = sendFifo.getData( &numBytes );
                delete [] nextData;
                }
            // else if -2, would block, try again next time
            }


            

        // try receiving next message

        if( nextIncomingMessageSize == 0 ) {
                
            // size bytes first
            unsigned char sizeBuffer[4];
            
            // non-block
            int numRecv = connectionSock->receive( sizeBuffer, 4, 0 );
            
                
            if( numRecv == -1 ) {
                // error
                netStatus = -1;
                printOut( "Receive error\n");
                return;
                }
            else if( numRecv == 4 ) {
                nextIncomingMessageSize =
                    sizeBuffer[0] << 24 |
                    sizeBuffer[1] << 16 |
                    sizeBuffer[2] << 8 |
                    sizeBuffer[3];

                // make sure size is sane
                if( nextIncomingMessageSize > 10000000 ) {
                    printOut( "Huge message size of %u received\n",
                              nextIncomingMessageSize );
                    netStatus = -1;
                    return;
                    }                


                nextIncomingMessage = 
                    new unsigned char[ nextIncomingMessageSize ];
                nextIncomingBytesRecievedSoFar = 0;
                }
            else if( numRecv == -2 ) {
                // would block, try again next time
                return;
                }
            }
        if( nextIncomingMessageSize != 0 ) {
            // got the size, need more of the body

            // non-block
            int numRecv = connectionSock->receive( 
                &( nextIncomingMessage[ nextIncomingBytesRecievedSoFar ] ), 
                nextIncomingMessageSize - nextIncomingBytesRecievedSoFar,
                0 );
            
                
            if( numRecv == -1 ) {
                // error
                netStatus = -1;
                printOut( "Receive error\n");
                return;
                }
            else if( numRecv > 0 ) {


                nextIncomingBytesRecievedSoFar += numRecv;
                
                if( nextIncomingBytesRecievedSoFar == 
                    nextIncomingMessageSize ) {
                    //printOut( "Got message\n" );
                    
                    // got it all

                    receiveFifo.addData( nextIncomingMessage, 
                                         nextIncomingMessageSize );

                    // prepare for next
                    nextIncomingMessageSize = 0;
                    delete [] nextIncomingMessage;
                    nextIncomingMessage = NULL;
                    nextIncomingBytesRecievedSoFar = 0;
                    }
                                    
                }
            // else -2, would block, try again
            
            }
        }
    }


void freeNetwork() {
    if( server != NULL ) {
        delete server;
        server = NULL;
        }
    if( connectionSock != NULL ) {
        delete connectionSock;
        connectionSock = NULL;
        }
    if( nextIncomingMessage != NULL ) {
        delete [] nextIncomingMessage;
        nextIncomingMessage = NULL;
        }    
    }





char isCameraSupported() {
    return true;
    }


void startCamera() {
    printOut( "Warning:  unsupported startCamera called on SDL platform\n" );
    }


void stopCamera() {
    printOut( "Warning:  unsupported stopCamera called on SDL platform\n" );
    }



void getFrame( unsigned char *inBuffer ) {
    int numPixels = 160*120;
    for( int i=0; i<numPixels; i++ ) {
        inBuffer[i] = getRandom( 255 );
        }
    
    //printOut( "Warning:  unsupported getFrame called on SDL platform\n" );
    }


void snapPicture( unsigned char *inBuffer ) {
    //printOut( "Warning:  unsupported snapPicture called on SDL platform\n" );
    int numPixels = 160*120;
    for( int i=0; i<numPixels; i++ ) {
        inBuffer[i] = getRandom( 255 );
        }
    }
