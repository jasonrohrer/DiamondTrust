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

#include "minorGems/graphics/openGL/glInclude.h"


#include "minorGems/util/stringUtils.h"



static const char *screenShotPrefix = "screen";

static void takeScreenShot();

static char shouldTakeScreenshot = false;



#ifdef USE_JPEG
    #include "minorGems/graphics/converters/JPEGImageConverter.h"
    static JPEGImageConverter screenShotConverter( 90 );
    static const char *screenShotExtension = "jpg";
#elif defined(USE_PNG)
    #include "minorGems/graphics/converters/PNGImageConverter.h"
    static PNGImageConverter screenShotConverter;
    static const char *screenShotExtension = "png";
#else
    #include "minorGems/graphics/converters/TGAImageConverter.h"
    static TGAImageConverter screenShotConverter;
    static const char *screenShotExtension = "tga";
#endif




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







static double channelVolume[MAX_SOUND_CHANNELS];
// smooth out volume changes given our large buffer size
static double lastReachedChannelVolume[MAX_SOUND_CHANNELS];
static double channelPan[MAX_SOUND_CHANNELS];

static char soundTryingToRun = false;


char isSoundTryingToRun() {
    return soundTryingToRun;
    }


char callbackCount = 0;

// don't make and destroy these every callback (slow!)
static int sampleBufferSize = 0;
static s16 *sumBufferL = NULL;
static s16 *sumBufferR = NULL;
static s16 *sampleBuffer = NULL;


static void freeAudioBuffers() {
    if( sumBufferL != NULL ) {
        delete [] sumBufferL;
        sumBufferL = NULL;
        }
    if( sumBufferR != NULL ) {
        delete [] sumBufferR;
        sumBufferR = NULL;
        }
    if( sampleBuffer != NULL ) {
        delete [] sampleBuffer;
        sampleBuffer = NULL;
        }
    }


void audioCallback( void *inUserData, Uint8 *inStream, int inLengthToFill ) {
    
    if( false && callbackCount < 10 ) {
        memset( inStream, 0, inLengthToFill );
        callbackCount ++;
        return;
        }
    


    unsigned int startMS = getSystemMilliseconds();
    

    soundTryingToRun = true;
    
    SDL_LockAudio();
    
    unsigned int netMSA = getSystemMilliseconds() - startMS;

    //printf( "Audio callback called\n" );
    
    // sum all 8 channels

    
    // stream has stereo, 16-bit samples
    int numSamplesToFill = inLengthToFill / 4;
    

    
    if( numSamplesToFill != sampleBufferSize ) {
        // need to make new buffers

        // destroy old?
        freeAudioBuffers();
        

        // each channel has mono, 16-bit samples
        sumBufferL = new s16[ numSamplesToFill ];
        sumBufferR = new s16[ numSamplesToFill ];
        
        sampleBuffer = new s16[ numSamplesToFill ];

        sampleBufferSize = numSamplesToFill;
        }    

    unsigned int netMSB = getSystemMilliseconds() - startMS;


    // clear to prepare for sum
    memset( sumBufferL, 0, numSamplesToFill * sizeof( s16 ) );
    memset( sumBufferR, 0, numSamplesToFill * sizeof( s16 ) );


    unsigned int netMSC = getSystemMilliseconds() - startMS;


    unsigned int channTimes[ MAX_SOUND_CHANNELS ];
    unsigned int channTimesB[ MAX_SOUND_CHANNELS ];

    for( int c=0; c<MAX_SOUND_CHANNELS; c++ ) {

        getAudioSamplesForChannel( c, sampleBuffer, numSamplesToFill );
        
        channTimes[c] = getSystemMilliseconds() - startMS;

        double targetVolume = channelVolume[c];
        double currentVolume = lastReachedChannelVolume[c];
        double pan = channelPan[c];
        
        // apply constant power law for stereo
        double p = M_PI * pan / 2;

        double rightVolume = sin( p );
        double leftVolume = cos( p );
        
        double volumeChangeRate = 
            ( targetVolume - currentVolume ) / numSamplesToFill;


        for( int s=0; s<numSamplesToFill; s++ ) {
            
            // avoid volume overflow with all channels together
            double monoSample = sampleBuffer[ s ] / (double)MAX_SOUND_CHANNELS;

            monoSample *= currentVolume;

            double leftSample = monoSample * leftVolume;
            double rightSample = monoSample * rightVolume;


            // fast float-to-int conversion
            sumBufferL[s] += (s16)lrint( leftSample );
            sumBufferR[s] += (s16)lrint( rightSample );

            currentVolume += volumeChangeRate;
            }

        // avoid round-off errors in sum
        lastReachedChannelVolume[c] = channelVolume[c];

        channTimesB[c] = getSystemMilliseconds() - startMS;
        }
    
    unsigned int netMSD = getSystemMilliseconds() - startMS;


    // now copy samples into Uint8 buffer
    int streamPosition = 0;
    for( int i=0; i != numSamplesToFill; i++ ) {
        Sint16 intSampleL = sumBufferL[i];
        Sint16 intSampleR = sumBufferR[i];
        
        inStream[ streamPosition ] = (Uint8)( intSampleL & 0xFF );
        inStream[ streamPosition + 1 ] = (Uint8)( ( intSampleL >> 8 ) & 0xFF );
        
        inStream[ streamPosition + 2 ] = (Uint8)( intSampleR & 0xFF );
        inStream[ streamPosition + 3 ] = (Uint8)( ( intSampleR >> 8 ) & 0xFF );
        
        streamPosition += 4;
        }

    unsigned int netMSE = getSystemMilliseconds() - startMS;

    
    SDL_UnlockAudio();


    soundTryingToRun = false;


    unsigned int netMS = getSystemMilliseconds() - startMS;
    
    if( netMS > 20 ) {
        printOut( "Audio callback took %d ms "
                  "(A:%d, B:%d, C:%d, (", netMS,
                  netMSA, netMSB, netMSC, netMSD );

        for( int c=0; c<MAX_SOUND_CHANNELS; c++ ) {
            printOut( "%d[%d,%d], ", c, channTimes[c], channTimesB[c] );
            }
        printOut( " ), E:%d\n", netMSE );
        
        }
    
    }



void cleanUpAtExit() {
    printf( "Exiting...\n" );

    SDL_CloseAudio();

    freeAudioBuffers();
    
    gameFree();    
    freeSprites();
    freeNetwork();
    }





int w = 256;
// two 192-high screens, 2 pixel boundary between
int h = 386;

int blowupFactor = 1;

int bottomScreenYOffset = 194;
int singleScreenH = 192;


// 30 fps
int frameMS = 33;
//int frameMS = 500;

// s and f keys to slow down and speed up for testing
//char enableSlowdownKeys = false;
char enableSlowdownKeys = true;


char touchWasDown = false;
char touchDown = false;
int touchX = 0;
int touchY = 0;



static Uint32 lastFrameDoneTime;

static Uint32 frameBatchStartTime;
    
static int frameBatchSize = 100;
static int frameBatchCount = 0;



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



    
    gameInit();



    SDL_AudioSpec audioFormat;

    /* Set 16-bit stereo audio at 22Khz */
    audioFormat.freq = 22050;
    audioFormat.format = AUDIO_S16;
    audioFormat.channels = 2;
    //audioFormat.samples = 8192;        /* avoid artifacts, ever */
    audioFormat.samples = 1024;        
    //audioFormat.samples = 512;        
    //audioFormat.samples = 2048;        
    audioFormat.callback = audioCallback;
    audioFormat.userdata = NULL;

    /* Open the audio device and start playing sound! */
    
    SDL_AudioSpec actualFormat;

    if( SDL_OpenAudio( &audioFormat, &actualFormat ) < 0 ) {
        printf( "Unable to open audio: %s\n", SDL_GetError() );
        }
    else {
        printf( "Opened audio with buffer size %d\n", actualFormat.samples );
        }
    

    // defaults
    for( int c=0; c<MAX_SOUND_CHANNELS; c++ ) {    
        channelVolume[c] = 0;
        lastReachedChannelVolume[c] = 0;
        channelPan[c] = 0.5;
        }
    


    // set pause to 0 to start audio
    SDL_PauseAudio(0);


    
    
    // to free frame drawer, stop audio, etc
    atexit( cleanUpAtExit );


    lastFrameDoneTime = SDL_GetTicks();

    frameBatchStartTime = SDL_GetTicks();
    
        
    

    
    // main loop
    while( true ) {
        runGameLoopOnce();
        }

    return 0;
    }




void runGameLoopOnce() {
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



void platformSleep( unsigned int inTargetMilliseconds ) {
    SDL_Delay( inTargetMilliseconds );
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

    glViewport( 0, blowupFactor * bottomScreenYOffset, 
                blowupFactor * w, blowupFactor * singleScreenH );
    
    
    spriteYOffset = 0;
    drawTopScreen();
    
    
	//spriteYOffset = bottomScreenYOffset;
    glViewport( 0, 0, 
                blowupFactor * w, blowupFactor * singleScreenH );
    drawBottomScreen();


    // separating line:

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, w, h, 0, -1.0f, 1.0f);
    
    
    glMatrixMode(GL_MODELVIEW);
    glViewport( 0, 0, 
                blowupFactor * w, blowupFactor * h );
    

    glColor4ub( 96, 96, 96, 255 );
    GLfloat lineVertices[] = {
        0, bottomScreenYOffset - 1,
        w * blowupFactor, bottomScreenYOffset - 1 
        };
    
    glVertexPointer( 2, GL_FLOAT, 0, lineVertices );
    glEnableClientState( GL_VERTEX_ARRAY );
    
        
    glDrawArrays( GL_LINES, 0, 2 );

    lineVertices[1] = bottomScreenYOffset - 2;
    lineVertices[3] = bottomScreenYOffset - 2;
    
    glDrawArrays( GL_LINES, 0, 2 );

    
    if( shouldTakeScreenshot ) {
        takeScreenShot();
        shouldTakeScreenshot = false;
        }
    

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
    
    if( inKey == '=' ) {
        shouldTakeScreenshot = true;
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

#include "../dataFiles.cpp"

unsigned char *readFile( const char *inFileName, int *outSize ) {
    
     // check if data for this file is inline-included
    unsigned char *inlinedData = readIncludedFile( inFileName, outSize );
    
    if( inlinedData != NULL ) {
        return inlinedData;
        }
        
    // else try other methods of reading file data
    


    // files are inside gameData directory


    File f( new Path( "gameData" ), inFileName );
    
    if( ! f.exists() ) {
        return NULL;
        }
    
    *outSize = f.getLength();
    
    return f.readFileContents( outSize, false );
    }



char isDirectory( const char *inFileName ) {
    File f( new Path( "gameData" ), inFileName );
    
    return f.isDirectory();
    }



char **listDirectory( const char *inFileName, int *outNumEntries ) {

    File f( new Path( "gameData" ), inFileName );

    if( ! f.isDirectory() ) {
        return NULL;
        }
    

    int numChildFiles;
    
    File **childFiles = f.getChildFiles( &numChildFiles );

    SimpleVector<char*> childNames;
    
    for( int i=0; i<numChildFiles; i++ ) {
        File *childFile = childFiles[i];
    
        char *childName = childFile->getFileName();

        // completely ignore "hidden" files
        // this avoids trouble caused by .DS_Store files created on Macs
        if( strlen( childName ) > 0 && childName[0] != '.' ) {
            
            char *childFullName =
                autoSprintf( "%s/%s", inFileName, childName );

            childNames.push_back( childFullName );
            }
        
        delete [] childName;


        delete childFile;
        }
    delete [] childFiles;
    
    *outNumEntries = childNames.size();
    
    return childNames.getElementArray();
    }



// decided that the ONLY way to make realtime file IO work in conjuction
// with audio streams is to slurp each necessary file into RAM before
// starting the audio thread.  Ugly in terms of RAM usage, but if we're only
// playing one song at a time, it's not that bad to pre-load all those loops.
//
// HOWEVER, this would be impossible on the DS due to RAM constraints, and
// is also not necessary because DS card reads have more consistent latency 
// than HD access.
//
// SO... we'll hide this RAM caching here, in the file reading interface.
// Whenever a file is OPEN, it has been cached in RAM.
//
// The music player already keeps all current loop files open from previous
// attempts at optimization.
//
// So, switching songs should involve CLOSING all the open files (to free
// RAM in the SDL implemenation) and then opening another batch of files.
// As long as the audio buffer has been set to silence before this happens,
// we should be fine even if opening the next batch of files takes a while.
// (On SDL, audio runs in a separate thread, so opening these files shouldn't
//  slow down the game... on the DS, opening the next batch of files shouldn't
//  be slow in the first place.)
typedef struct FileStream {
        int fileSize;
        
        int filePosition;
        
        unsigned char *fileData;
        
    } FileStream;




FileHandle openFile( const char *inFileName, int *outSize ) {

    FileStream *stream = new FileStream;
    
    stream->fileData = readFile( inFileName, &( stream->fileSize ) );

    if( stream->fileData == NULL ) {
        delete stream;
        return NULL;
        }
    
    stream->filePosition = 0;
    
    *outSize = stream->fileSize;

    return stream;
    }



int readFile( FileHandle inFile, unsigned char *inBuffer, int inBytesToRead ) {
    FileStream *stream = (FileStream*)inFile;

    int bytesLeft = stream->fileSize - stream->filePosition;
    
    if( bytesLeft < inBytesToRead ) {
        inBytesToRead = bytesLeft;
        }
    
    memcpy( inBuffer, &( stream->fileData[ stream->filePosition ] ),
            inBytesToRead );
    
    stream->filePosition += inBytesToRead;
    
    return inBytesToRead;
    }


void closeFile( FileHandle inFile ) {
    FileStream *stream = (FileStream*)inFile;
    
    delete [] stream->fileData;
    
    delete stream;
    }



void fileSeek( FileHandle inFile, int inAbsolutePosition ) {
    FileStream *stream = (FileStream*)inFile;

    stream->filePosition = inAbsolutePosition;
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



#include "minorGems/system/Time.h"

static char baseTimeSet = false;
static unsigned long baseSeconds, baseMilliseconds;

unsigned int getSystemMilliseconds() {
    
    if( !baseTimeSet ) {
        Time::getCurrentTime( &baseSeconds, &baseMilliseconds );
        baseTimeSet = true;
        return 0;
        }

    return Time::getMillisecondsSince( baseSeconds, baseMilliseconds );
    }




#include "minorGems/util/SimpleVector.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"


typedef struct texturePalette {
        rgbaColor fullColorPalette[256];
    } texturePalette;



typedef struct sprite {
        SingleTextureGL *texture;
        unsigned int w;
        unsigned int h;
    } sprite;



SimpleVector<sprite> spriteInfo;

// save palettes to be reused if 8-bit texture data changes
// these will only contain placeholders for non-8-bit sprites
SimpleVector<texturePalette> spritePalettes;


void freeSprites() {
    for( int i=0; i<spriteInfo.size(); i++ ) {
        SingleTextureGL *t = spriteInfo.getElement( i )->texture;
        delete t;
        }
    }


static int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight,
                      texturePalette *inP ) {
    
    SingleTextureGL *s = new SingleTextureGL( (unsigned char*)inDataRGBA,
                                              inWidth, inHeight );
    sprite thisSprite = { s, inWidth, inHeight };
    
    spriteInfo.push_back( thisSprite );
        
    spritePalettes.push_back( *inP );

    return spriteInfo.size() - 1;
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
    
    SingleTextureGL *s = spriteInfo.getElement( inSpriteID )->texture;

    s->replaceTextureData( (unsigned char*)fullColorImage,
                           false, inWidth, inHeight );

    delete [] fullColorImage;
    }





static const GLfloat squareTextureCoords[] = {
    0, 0,
    1, 0,
    0, 1,
    1, 1
    };



void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor ) {
    glColor4ub( inColor.r, inColor.g, inColor.b, inColor.a );
        
    sprite s = *( spriteInfo.getElementFast( inHandle ) );
    
    SingleTextureGL *texture = s.texture;

    texture->enable();

    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );



    glTexCoordPointer( 2, GL_FLOAT, 0, squareTextureCoords );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    
    // account for "screen" being drawn on
    inY += spriteYOffset;
        
    int xEnd = inX + s.w;
    int yEnd = inY + s.h;
    
    const GLfloat squareVertices[] = {
        inX, inY,
        xEnd, inY,
        inX, yEnd,
        xEnd, yEnd };    
    
    glVertexPointer( 2, GL_FLOAT, 0, squareVertices );
    glEnableClientState( GL_VERTEX_ARRAY );
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    
    texture->disable();
    }






void drawSprite( int inHandle, int inNumCopies, 
                 int inX[], int inY[], rgbaColor inColor ) {
    glColor4ub( inColor.r, inColor.g, inColor.b, inColor.a );

    sprite s = *( spriteInfo.getElementFast( inHandle ) );
    
    SingleTextureGL *texture = s.texture;

    texture->enable();
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );



    glTexCoordPointer( 2, GL_FLOAT, 0, squareTextureCoords );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );


    for( int i=0; i<inNumCopies; i++ ) {
        int x = inX[i];
        int y = inY[i];
        
        // account for "screen" being drawn on
        y += spriteYOffset;
        
        int xEnd = x + s.w;
        int yEnd = y + s.h;

        const GLfloat squareVertices[] = {
            x, y,
            xEnd, y,
            x, yEnd,
            xEnd, yEnd };    

        glVertexPointer( 2, GL_FLOAT, 0, squareVertices );
        glEnableClientState( GL_VERTEX_ARRAY );
    
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    
    texture->disable();
    }




void startNewSpriteLayer() {
    // no layering necessary in SDL version
    }












static int nextShotNumber = -1;
static char shotDirExists = false;


void takeScreenShot() {
     
    
    File shotDir( NULL, "screenShots" );
    
    if( !shotDirExists && !shotDir.exists() ) {
        shotDir.makeDirectory();
        shotDirExists = shotDir.exists();
        }
    
    if( nextShotNumber < 1 ) {
        if( shotDir.exists() && shotDir.isDirectory() ) {
        
            int numFiles;
            File **childFiles = shotDir.getChildFiles( &numFiles );

            nextShotNumber = 1;

            char *formatString = autoSprintf( "%s%%d.%s", screenShotPrefix,
                                              screenShotExtension );

            for( int i=0; i<numFiles; i++ ) {
            
                char *name = childFiles[i]->getFileName();
                
                int number;
                
                int numRead = sscanf( name, formatString, &number );
                
                if( numRead == 1 ) {
                    
                    if( number >= nextShotNumber ) {
                        nextShotNumber = number + 1;
                        }
                    }
                delete [] name;
                
                delete childFiles[i];
                }
            
            delete [] formatString;
            
            delete [] childFiles;
            }
        }
    

    if( nextShotNumber < 1 ) {
        return;
        }
    
    char *fileName = autoSprintf( "%s%05d.%s", 
                                  screenShotPrefix, nextShotNumber,
                                  screenShotExtension );

    

    File *file = shotDir.getChildFile( fileName );
    
    delete [] fileName;


    int numBytes = w * h * 3;
    
    unsigned char *rgbBytes = 
        new unsigned char[ numBytes ];

    // w and h might not be multiples of 4
    GLint oldAlignment;
    glGetIntegerv( GL_PACK_ALIGNMENT, &oldAlignment );
                
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    
    glReadPixels( 0, 0, w, h, 
                  GL_RGB, GL_UNSIGNED_BYTE, rgbBytes );
    
    glPixelStorei( GL_PACK_ALIGNMENT, oldAlignment );

    nextShotNumber++;
    
    

    Image screenImage( w, h, 3, false );

    double *channels[3];
    int c;
    for( c=0; c<3; c++ ) {
        channels[c] = screenImage.getChannel( c );
        }
    
    // image of screen is upside down
    int outputRow = 0;
    for( int y=h-1; y>=0; y-- ) {
        for( int x=0; x<w; x++ ) {
                        
            int outputPixelIndex = outputRow * w + x;
            
            
            int screenPixelIndex = y * w + x;
            int byteIndex = screenPixelIndex * 3;
                        
            for( c=0; c<3; c++ ) {
                channels[c][outputPixelIndex] =
                    rgbBytes[ byteIndex + c ] / 255.0;
                }
            }
        outputRow++;
        }
    
    delete [] rgbBytes;


    FileOutputStream tgaStream( file );
    
    screenShotConverter.formatImage( &screenImage, &tgaStream );

    delete file;
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


int getSignalStrength() {
    // never show signal strength on SDL
    return -1;
    }



char isAutoconnecting() {
    return false;
    }


char shouldShowDSWiFiIcons() {
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



void closeConnection() {
    if( server != NULL ) {
        delete server;
        server = NULL;
        }
    if( connectionSock != NULL ) {
        delete connectionSock;
        connectionSock = NULL;
        }
    netStatus = 0;
    }





#include "DataFifo.h"

DataFifo sendFifo;
DataFifo receiveFifo;

unsigned int nextIncomingMessageSize = 0;
unsigned char nextIncomingMessageChannel = 0;
unsigned char *nextIncomingMessage = NULL;
unsigned int nextIncomingBytesRecievedSoFar = 0;



void sendMessage( unsigned char *inMessage, unsigned int inLength,
                  unsigned char inChannel ) {
    
    if( netStatus != 1 ) {
        printOut( "sendMessage called when network not running.  "
                  "Discarding message\n" );
        return;
        }

    
    sendFifo.addData( inMessage, inLength, inChannel );
    }

void sendLastMessage( unsigned char *inMessage, unsigned int inLength,
                      unsigned char inChannel ) {
    sendFifo.addData( inMessage, inLength, inChannel, true );
    }


unsigned char *getMessage( unsigned int *outLength, unsigned char inChannel ) {
    unsigned char channel;

    return receiveFifo.getData( outLength, true, inChannel, &channel );
    }



void stepNetwork() {
    if( netStatus == 0 ) {
        
        if( server != NULL ) {
        
            char timedOut;
            
            connectionSock = server->acceptConnection( 0, &timedOut );
            
            if( connectionSock != NULL ) {
                printOut( "Got connection\n" );
                
                netStatus = 1;

                // only one connection at a time
                // don't need to keep server around
                delete server;
                server = NULL;
                }
            else if( !timedOut ) {
                // error
                netStatus = -1;
                printOut( "Incoming connection error\n");

                delete server;
                server = NULL;
                }
            }        
        else if( connectionSock != NULL ) {
            if( netStatus == 0 ) {
                // check if connected
                netStatus = connectionSock->isConnected();
                
                if( netStatus == -1 ) {
                    printOut( "Error connecting to server\n");
                    delete connectionSock;
                    connectionSock = NULL;
                    }
                }
            }
        }
    else if( netStatus == 1 ) {

        // try sending next message
        unsigned int numBytes;
        unsigned char channel;
        char lastMessageFlag;
        
        unsigned char *nextData = sendFifo.peekData( &numBytes, false, 0,
                                                     &channel, 
                                                     &lastMessageFlag );
            
        if( nextData != NULL ) {
            // first 5 bytes are size and channel
            unsigned int sendSize = numBytes + 5;
            unsigned char *sendData = new unsigned char[ sendSize ];
                
            sendData[0] = ( numBytes >> 24 ) & 0xFF;
            sendData[1] = ( numBytes >> 16 ) & 0xFF;
            sendData[2] = ( numBytes >> 8 ) & 0xFF;
            sendData[3] = ( numBytes ) & 0xFF;
            sendData[4] = channel;
            
            memcpy( &( sendData[5] ), nextData, numBytes );
                
            delete [] nextData;
                
            // non-blocking
            int numSent = 
                connectionSock->send( sendData, sendSize, false );

            
            delete [] sendData;
                
            if( numSent == -1 ) {
                // error
                netStatus = -1;
                printOut( "Send error\n");
                
                delete connectionSock;
                connectionSock = NULL;
                
                return;
                }
            else if( numSent == (int)sendSize ) {
                // done
                    
                // remove from fifo
                nextData = sendFifo.getData( &numBytes, false, 0, &channel );
                delete [] nextData;

                if( lastMessageFlag ) {
                    closeConnection();
                    return;
                    }
                }
            // else if -2, would block, try again next time
            }


            

        // try receiving next message

        if( nextIncomingMessageSize == 0 ) {
                
            // size and channel bytes first
            unsigned char headerBuffer[5];
            
            // non-block
            int numRecv = connectionSock->receive( headerBuffer, 5, 0 );
            
                
            if( numRecv == -1 ) {
                // error
                netStatus = -1;
                printOut( "Receive error\n");
                
                delete connectionSock;
                connectionSock = NULL;

                return;
                }
            else if( numRecv == 5 ) {
                nextIncomingMessageSize =
                    headerBuffer[0] << 24 |
                    headerBuffer[1] << 16 |
                    headerBuffer[2] << 8 |
                    headerBuffer[3];
                nextIncomingMessageChannel = headerBuffer[4];
                
                // make sure size is sane
                if( nextIncomingMessageSize > 10000000 ) {
                    printOut( "Huge message size of %u received\n",
                              nextIncomingMessageSize );
                    netStatus = -1;

                    delete connectionSock;
                    connectionSock = NULL;

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

                delete connectionSock;
                connectionSock = NULL;
                
                return;
                }
            else if( numRecv > 0 ) {


                nextIncomingBytesRecievedSoFar += numRecv;
                
                if( nextIncomingBytesRecievedSoFar == 
                    nextIncomingMessageSize ) {
                    //printOut( "Got message\n" );
                    
                    // got it all

                    receiveFifo.addData( nextIncomingMessage, 
                                         nextIncomingMessageSize,
                                         nextIncomingMessageChannel );

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
    return false;
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





// these do nothing for non-DS platforms

char isCloneBootPossible() {
    return false;
    }


char isThisAClone() {
    // never a clone on SDL
    return false;
    }


void acceptCloneDownloadRequest() {
    }


int getCloneHostState() {
    return -1;
    }


const char *getCloneChildUserName() {
    return "";
    }



void cancelCloneHosting() {
    }



void checkCloneFetch() {
    }







void setSoundChannelVolume( int inChannelNumber, int inVolume ) {
    SDL_LockAudio();
    
    channelVolume[inChannelNumber] = inVolume / 127.0;
    
    SDL_UnlockAudio();
    }


void setSoundChannelPan( int inChannelNumber, int inPan ) {
    SDL_LockAudio();

    channelPan[inChannelNumber] = inPan / 127.0;

    SDL_UnlockAudio();
    }



void lockAudio() {
    SDL_LockAudio();
    }



void unlockAudio() {
    SDL_UnlockAudio();
    }

