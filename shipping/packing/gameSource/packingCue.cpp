#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"

#include "minorGems/game/Font.h"

#include "minorGems/util/random/CustomRandomSource.h"



Font *mainFont;
Font *displayFont;
Font *displayFontFixed;
// position of view in world
doublePair lastScreenViewCenter = {0, 0 };



// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;

double frameRateFactor = 1;



unsigned int randSeed = 1285702442;
CustomRandomSource randSource(randSeed);



#define NUM_SPECIALS 6

SpriteHandle specialPictures[ NUM_SPECIALS ];


const char *specialTGAFiles[ NUM_SPECIALS ] = { "largeDiamond.tga",
                                                "angolanStamp.tga",
                                                "travellersCheck.tga",
                                                "angolanBill.tga",
                                                "belgianCoin.tga",
                                                "extraDiamond.tga" };


int specialCounts[ NUM_SPECIALS ] = { 161,
                                      403,
                                      78,
                                      28,
                                      63,
                                      781 };

// specialCounts[s][e] true if special s occurs in envelope e
char specialDistributions[ NUM_SPECIALS ][1000];

                                                


int currentDisplay = 1;





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
    displayFont = new Font( getFontTGAFileName(), 1, 4, false, 2 );
    displayFontFixed = new Font( getFontTGAFileName(), 0, 1, true, 2 );


    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );

    viewHeightFraction = inHeight / (double)inWidth;

    // monitors vary in width relative to height
    // keep visible vertical view span constant (15)
    // which is what it would be for a view width of 20 at a 4:3 aspect
    // ratio
    viewWidth = 15 * 1.0 / viewHeightFraction;
    
    
    setViewSize( viewWidth );


    // init
    for( int i=0; i<NUM_SPECIALS; i++ ) {
        specialPictures[i] = loadSprite( specialTGAFiles[i], false );

        for( int e=0; e<1000; e++ ) {
            specialDistributions[i][e] = false;
            }
        }
    



    // setup distributions

    // place all but extra regular diamonds totally randomly 
    for( int i=0; i<NUM_SPECIALS - 1; i++ ) {
        int countPlaced = 0;
        

        while( countPlaced < specialCounts[i] ) {
            
            int pick = randSource.getRandomBoundedInt( 0, 999 );
            
            
            while( specialDistributions[i][pick] ) {
                // already full, pick again
                pick = randSource.getRandomBoundedInt( 0, 999 );
                }
            
            // found empty, fill
            specialDistributions[i][pick] = true;
            
        
            countPlaced++;
            }
        

        printf( "\n\n\nPlacement for special %d:\n", i );
        
        for( int e=0; e<1000; e++ ) {
            printf( "%d ", specialDistributions[i][e] );
            }

        int countLowerHalf = 0;
        for( int e=0; e<500; e++ ) {
            if( specialDistributions[i][e] ) {
                countLowerHalf ++;
                }
            }
        printf( "%d%% in lower half\n\n", 
                (countLowerHalf * 100)/ specialCounts[i] );

        }





    // now start by placing regular diamonds in EVERY slot that 
    // has nothing in it
    int j = NUM_SPECIALS - 1;
    int countPlaced = 0;
    for( int e=0; e<1000; e++ ) {
        char thisEmpty = true;

        for( int i=0; i<NUM_SPECIALS; i++ ) {
            if( specialDistributions[i][e] ) {
                thisEmpty = false;
                break;
                }
            }

        if( thisEmpty && countPlaced < specialCounts[j] ) {
            specialDistributions[j][e] = true;
            countPlaced ++;
            }
        }
    
    // now place rest randomly
    while( countPlaced < specialCounts[j] ) {
            
        int pick = randSource.getRandomBoundedInt( 0, 999 );
            
            
        while( specialDistributions[j][pick] ) {
            // already full, pick again
            pick = randSource.getRandomBoundedInt( 0, 999 );
            }
            
        // found empty, fill
        specialDistributions[j][pick] = true;
        
        
        countPlaced++;
        }
    




    printf( "\nDone placing\n" );
    
    int totallyEmpty = 0;
    
    for( int e=0; e<1000; e++ ) {
        char thisEmpty = true;

        for( int i=0; i<NUM_SPECIALS; i++ ) {
            if( specialDistributions[i][e] ) {
                thisEmpty = false;
                break;
                }
            }

        if( thisEmpty ) {
            totallyEmpty ++;
            }
        }
    printf( "%d envelopes have no special items.\n", totallyEmpty );


    }



void freeFrameDrawer() {
    delete mainFont;
    delete displayFont;
    delete displayFontFixed;
    
    for( int i=0; i<NUM_SPECIALS; i++ ) {
        freeSprite( specialPictures[i] );
        }
    
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
    

    double viewHeight = viewHeightFraction * viewWidth;

    doublePair messagePos = lastScreenViewCenter;


    setDrawColor( 1, 1, 1, 1 );

    messagePos = lastScreenViewCenter;

    messagePos.y += 5.75 * ( viewHeight / 15 );

    
    char *message = autoSprintf( "%d", currentDisplay );

    displayFont->drawString( "Envelope:", messagePos, alignRight );

    setDrawColor( 1, 1, 0.25, 1 );


    messagePos.x += 5;
    displayFontFixed->drawString( message, messagePos, alignRight );    

    delete [] message;
    

    setDrawColor( 1, 1, 1, 1 );


    doublePair picPosition = lastScreenViewCenter;
    
    picPosition.x -= 6;
    picPosition.y += 2;
    for( int i=0; i<NUM_SPECIALS; i++ ) {

        if( i == NUM_SPECIALS / 2 ) {
            picPosition.y -= 5;
            picPosition.x = lastScreenViewCenter.x - 6;
            }
        
        if( specialDistributions[i][ currentDisplay ] ) {
    
            drawSprite( specialPictures[i], picPosition, 0.5/16.0 );
            }
        picPosition.x += 5;
        }
    


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
    if( inASCII == ' ' ) {
        currentDisplay ++;
        if( currentDisplay > 1000 ) {
            currentDisplay = 1000;
            }
        }
    else if( inASCII == 'w' || inASCII == 'W' ) {
        currentDisplay += 100;
        currentDisplay = ( currentDisplay / 100 ) * 100;
        if( currentDisplay > 1000 ) {
            currentDisplay = 1000;
            }
        }
    else if( inASCII == 's' || inASCII == 'S' ) {
        currentDisplay -= 100;
        currentDisplay = ( currentDisplay / 100 ) * 100;
        if( currentDisplay < 1 ) {
            currentDisplay = 1;
            }
        }
    }


void keyUp( unsigned char inASCII ) {
    }




void specialKeyDown( int inKeyCode ) {
    if( inKeyCode == MG_KEY_RIGHT ) {
        currentDisplay ++;
        if( currentDisplay > 1000 ) {
            currentDisplay = 1000;
            }
        }
    else if( inKeyCode == MG_KEY_LEFT ) {
        currentDisplay --;
        if( currentDisplay < 1 ) {
            currentDisplay = 1;
            }
        }
    if( inKeyCode == MG_KEY_PAGE_UP ) {
        currentDisplay += 10;
        currentDisplay = ( currentDisplay / 10 ) * 10;
        if( currentDisplay > 1000 ) {
            currentDisplay = 1000;
            }
        }
    else if( inKeyCode == MG_KEY_PAGE_DOWN ) {
        currentDisplay -= 10;
        currentDisplay = ( currentDisplay / 10 ) * 10;
        if( currentDisplay < 1 ) {
            currentDisplay = 1;
            }
        }
    }



void specialKeyUp( int inKeyCode ) {
    }








// should sound be initialized?
char getUsesSound() {
    return false;
    }


void getSoundSamples( Uint8 *inBuffer, int inLengthToFillInBytes ) {
    }

