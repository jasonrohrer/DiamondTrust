#include "help.h"
#include "Button.h"
#include "greenBarPaper.h"
#include "gameStats.h"
#include "common.h"


#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"


extern Button *helpButton;

extern Font *font8;


extern int greenBarLeftMargin;



// starts off bottom of screen
static int helpGreenbarPageTop = 192;


static char helpShouldShow = false;


// for accel
static int helpPageCurrentSpeed = 0;


static const char *transKey = NULL;

SimpleVector<char *> helpLines;



char isHelpShowing() {
    if( helpGreenbarPageTop < 192 ) {
        return true;
        }
    
    // not on screen, but maybe it's trying to come up
    return helpShouldShow;
    }



char isHelpTryingToScroll() {
    if( ( helpShouldShow && helpGreenbarPageTop > 0 ) 
        ||
        ( !helpShouldShow && helpGreenbarPageTop < 192 ) ) {
        return true;
        }
    
    return false;
    }
    



static void clearOldLines() {
    for( int i=0; i<helpLines.size(); i++ ) {
        delete [] *( helpLines.getElement( i ) );
        }
    helpLines.deleteAll();
    }



void freeHelp() {
    clearOldLines();
    }



void showHelp( const char *inHelpTransKey ) {
    helpShouldShow = true;
    
    transKey = inHelpTransKey;


    clearOldLines();


    // compose new lines
    
    const char *helpText = translate( (char*)transKey );
    
    int lineNumber = 0;



    SimpleVector<char*> *words = tokenizeString( helpText );


    int spaceWidth = font8->measureString( " " );
    int charSpacing = font8->getCharacterSpacing();
    

    int wordIndex = 0;
    int numWords = words->size();
    
    while( wordIndex < numWords ) {
    
        int currentLineWidth = 0;
        

        int allowedSpace = 220;
        
        if( lineNumber == 10 || lineNumber == 11 ) {
            // leave room for help button in lower right corner
            allowedSpace = 213;
            }

        SimpleVector<char> currentLine;
        

        char overflow = false;
        
        char firstWord = true;

        while( !overflow && wordIndex < numWords ) {
            
            char *nextWord = *( words->getElement( wordIndex ) );
            
            int wordWidth = font8->measureString( nextWord );
           
            
            if( !firstWord ) {
                // spacing between last word and this word

                wordWidth += spaceWidth;
                wordWidth += charSpacing + charSpacing;
                }
            

            if( currentLineWidth + wordWidth > allowedSpace ) {
                overflow = true;
                }
            else {
                if( ! firstWord ) {
                    // space to separate from previous word
                    currentLine.push_back( ' ' );
                    }
                
                currentLine.push_back( nextWord, (int)strlen( nextWord ) );
            
                currentLineWidth += wordWidth;

                delete [] nextWord;
                wordIndex ++;
                }

            firstWord = false;
            }
        
        char *lineString = currentLine.getElementString();
        
        helpLines.push_back( lineString );

        lineNumber++;
        }
    
    delete words;

    }


void forceHideHelp() {
    helpShouldShow = false;
    }



void clickHelp( int inX, int inY ) {
    if( helpButton->getPressed( inX, inY ) ) {
        // toggle
        helpShouldShow = ! helpShouldShow;
        }
    }




void stepHelp() {
    int scrollRate = 8;

    if( helpShouldShow && helpGreenbarPageTop > 0 ) {
        if( helpGreenbarPageTop < 20 ) {
            // slow down near top
            if( helpPageCurrentSpeed < -2 ) {
                helpPageCurrentSpeed += 2;
                }
            }
        else if( helpPageCurrentSpeed > -scrollRate ) {
            helpPageCurrentSpeed -= 2;
            }

        helpGreenbarPageTop += helpPageCurrentSpeed;
        
        if( helpGreenbarPageTop == 0 ) {
            // hit top
            helpPageCurrentSpeed = 0;
            }
        }
    else if( !helpShouldShow && helpGreenbarPageTop < 192 ) {
        if( helpGreenbarPageTop > 172 ) {
            // slow down near bottom
            if( helpPageCurrentSpeed > 2 ) {
                helpPageCurrentSpeed -= 2;
                }
            }
        else if( helpPageCurrentSpeed < scrollRate ) {
            helpPageCurrentSpeed += 2;
            }

        helpGreenbarPageTop += helpPageCurrentSpeed;

        if( helpGreenbarPageTop == 192 ) {
            // hit bottom
            helpPageCurrentSpeed = 0;
            }
        }
    }

    

void drawHelp() {
    drawGreenBarPaper( helpGreenbarPageTop, 192 );
    
    startNewSpriteLayer();
    
    int lineNumber = 0;

    int lineOffset = 5;


    for( int i=0; i<helpLines.size(); i++ ) {
        char *lineString = *( helpLines.getElement( i ) );
        
        int lineY = lineNumber * 16 + lineOffset;
        
        int lineScreenY = helpGreenbarPageTop + lineY;

        if( lineScreenY > 192 ) {
            // off bottom of screen, don't draw any more lines
            break;
            }

        font8->drawString( lineString, greenBarLeftMargin,
                           lineScreenY,
                           getGreenBarInkColor( lineY, 
                                                getMonthsLeft(), 
                                                true ),
                           alignLeft );
        
        lineNumber++;
        }

    startNewSpriteLayer();
    
    helpButton->draw();
    }
