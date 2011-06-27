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





char isHelpShowing() {
    if( helpGreenbarPageTop < 192 ) {
        return true;
        }
    
    // not on screen, but maybe it's trying to come up
    return helpShouldShow;
    }



void showHelp( const char *inHelpTransKey ) {
    helpShouldShow = true;
    
    transKey = inHelpTransKey;
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


    char *helpText = translate( (char*)transKey );
    
    int lineNumber = 0;

    int lineOffset = 21;

    SimpleVector<char*> *words = tokenizeString( helpText );


    int spaceWidth = font8->measureString( " " );
    int charSpacing = font8->getCharacterSpacing();
    

    int wordIndex = 0;
    int numWords = words->size();
    
    while( wordIndex < numWords ) {
    
        int currentLineWidth = 0;


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
            

            if( currentLineWidth + wordWidth > 220 ) {
                overflow = true;
                }
            else {
                if( ! firstWord ) {
                    // space to separate from previous word
                    currentLine.push_back( ' ' );
                    }
                
                currentLine.push_back( nextWord, strlen( nextWord ) );
            
                currentLineWidth += wordWidth;

                delete [] nextWord;
                wordIndex ++;
                }

            firstWord = false;
            }
        
        char *lineString = currentLine.getElementString();
        

        int lineY = lineNumber * 16 + lineOffset;
        
        font8->drawString( lineString, greenBarLeftMargin,
                           helpGreenbarPageTop + lineY,
                           getGreenBarInkColor( lineY, 
                                                getMonthsLeft(), 
                                                false ),
                           alignLeft );
        
        delete [] lineString;
        
        lineNumber++;
        }
    
    delete words;
    


    
    helpButton->draw();
    }
