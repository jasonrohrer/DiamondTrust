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
        helpGreenbarPageTop -= scrollRate;
        }
    else if( !helpShouldShow && helpGreenbarPageTop < 192 ) {
        helpGreenbarPageTop += scrollRate;
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
    

    int wordIndex = 0;
    int numWords = words->size();
    
    while( wordIndex < numWords ) {
    
        int currentLineWidth = 0;


        SimpleVector<char> currentLine;
        

        char overflow = false;
        
        while( !overflow && wordIndex < numWords ) {
            
            char *nextWord = *( words->getElement( wordIndex ) );
            
            int wordWidth = font8->measureString( nextWord );
            wordWidth += spaceWidth;

            if( currentLineWidth + wordWidth > 220 ) {
                overflow = true;
                }
            else {
                currentLine.push_back( nextWord, strlen( nextWord ) );
                currentLine.push_back( ' ' );
            
                currentLineWidth += wordWidth;

                delete [] nextWord;
                wordIndex ++;
                }
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
