#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"

#include "minorGems/game/Font.h"




Font *mainFont;
// position of view in world
doublePair lastScreenViewCenter = {0, 0 };



// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;

double frameRateFactor = 1;




const char *getWindowTitle() {
    return "Packing Cue";
    }



char *getCustomRecordedGameData() {
    return stringDuplicate( "custom" );
    }


char showMouseDuringPlayback() {
    return true;
    }


char *getHashSalt() {
    return stringDuplicate( "packing" );
    }




// name of custom font TGA file to find in "graphics" folder
const char *getFontTGAFileName() {
    return "font_8_16.tga";
    }


// called by platform to draw status messages on top of game image
// game can pick where these should be displayed (though platform ensures
// that it is drawn after frame is drawn)
// String might contain multiple lines separated by '\n'
void drawString( const char *inString ) {
    setDrawColor( 1, 1, 1, 0.75 );

    doublePair messagePos = lastScreenViewCenter;

    messagePos.x -= viewWidth / 2;
        
    messagePos.x +=  0.25;
    

    
    messagePos.y += (viewWidth * viewHeightFraction) /  2;
    
    messagePos.y -= 0.4375;
    messagePos.y -= 0.5;
    

    int numLines;
    
    char **lines = split( inString, "\n", &numLines );
    
    for( int i=0; i<numLines; i++ ) {
        

        mainFont->drawString( lines[i], messagePos, alignLeft );
        messagePos.y -= 0.75;
        
        delete [] lines[i];
        }
    delete [] lines;
    }



char isDemoMode() {
    return false;
    }



const char *getDemoCodeSharedSecret() {
    return "nothing";
    }


const char *getDemoCodeServerURL() {
    return "none";
    }



void initFrameDrawer( int inWidth, int inHeight, int inTargetFrameRate,
                      const char *inCustomRecordedGameData,
                      char inPlayingBack ) {
    
    if( inTargetFrameRate != 60 ) {
        frameRateFactor = (double)60 / (double)inTargetFrameRate;
        }

    printf( "Game playing back %d\n", inPlayingBack );
    

    mainFont = new Font( getFontTGAFileName(), 1, 4, false );


    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );

    viewHeightFraction = inHeight / (double)inWidth;

    // monitors vary in width relative to height
    // keep visible vertical view span constant (15)
    // which is what it would be for a view width of 20 at a 4:3 aspect
    // ratio
    viewWidth = 15 * 1.0 / viewHeightFraction;
    
    
    setViewSize( viewWidth );
    }



void freeFrameDrawer() {
    delete mainFont;
    }





static float pauseScreenFade = 0;

static void drawPauseScreen() {

    double viewHeight = viewHeightFraction * viewWidth;

    setDrawColor( 1, 1, 1, 0.5 * pauseScreenFade );
        
    drawSquare( lastScreenViewCenter, 1.05 * ( viewHeight / 3 ) );
        

    setDrawColor( 0.2, 0.2, 0.2, 0.85 * pauseScreenFade  );
        
    drawSquare( lastScreenViewCenter, viewHeight / 3 );
        

    setDrawColor( 1, 1, 1, pauseScreenFade );

    doublePair messagePos = lastScreenViewCenter;

    messagePos.y += 4.5;

    mainFont->drawString( translate( "pauseMessage1" ), 
                           messagePos, alignCenter );


    setDrawColor( 1, 1, 1, pauseScreenFade );

    messagePos = lastScreenViewCenter;

    messagePos.y -= 3.75 * ( viewHeight / 15 );
    //mainFont->drawString( translate( "pauseMessage3" ), 
    //                      messagePos, alignCenter );

    messagePos.y -= 0.625 * (viewHeight / 15);
    mainFont->drawString( translate( "pauseMessage3" ), 
                           messagePos, alignCenter );

    }





void drawFrame( char inUpdate ) {



    if( pauseScreenFade > 0 ) {
        drawPauseScreen();
        }
    

    if( !inUpdate ) {
    
        // fade in pause screen
        if( pauseScreenFade < 1 ) {
            pauseScreenFade += ( 1.0 / 30 ) * frameRateFactor;
        
            if( pauseScreenFade > 1 ) {
                pauseScreenFade = 1;
                }
            }
        return;
        }
    
    // fade pause screen out
    if( pauseScreenFade > 0 ) {
        pauseScreenFade -= ( 1.0 / 30 ) * frameRateFactor;
        
        if( pauseScreenFade < 0 ) {
            pauseScreenFade = 0;
            }
        }
    
    }






void pointerMove( float inX, float inY ) {
    }


void pointerDown( float inX, float inY ) {
    }


void pointerDrag( float inX, float inY ) {
    }



void pointerUp( float inX, float inY ) {
    }



void keyDown( unsigned char inASCII ) {
    }


void keyUp( unsigned char inASCII ) {
    }




void specialKeyDown( int inKeyCode ) {
    }


void specialKeyUp( int inKeyCode ) {
    }








// should sound be initialized?
char getUsesSound() {
    return false;
    }


void getSoundSamples( Uint8 *inBuffer, int inLengthToFillInBytes ) {
    }

