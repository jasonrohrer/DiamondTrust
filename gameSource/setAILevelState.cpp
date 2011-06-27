#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "salePicker.h"
#include "colors.h"
#include "ai/ai.h"
#include "greenBarPaper.h"
#include "gameStats.h"

#include "minorGems/util/stringUtils.h"



static char stateDone = false;

extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;

extern int aiLevelSheetSpriteID;


extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;

extern Font *font16;


static char waitingForDone = false;



class SetAILevelState : public GameState {
    public:
        SetAILevelState();
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        virtual char needsNextButton() {
            // auto-advance
            return false;
            }

        virtual char canStateBeBackedOut() {
            return true;
            }

        
        virtual char canShowHelp() {
            return true;
            }
        
        virtual const char *getHelpTransKey() {
            return "help_aiLevel";
            }


        
        virtual ~SetAILevelState();
        
    protected:
        char *mMessage;
    };


SetAILevelState::SetAILevelState() 
        : mMessage( NULL ) {
    mStateName = "SetAILevelState";
    }

SetAILevelState::~SetAILevelState() {
    
    if( mMessage != NULL ) {
        delete [] mMessage;
        }
    }




void SetAILevelState::clickState( int inX, int inY ) {
    if( waitingForDone ) {
        int oldSetting = getPickerSale();
        
        
        clickSalePicker( inX, inY );
        
        
        int newSetting = getPickerSale();
        
        if( newSetting > 1000 ) {
            // too many!  stop
            newSetting = oldSetting;
            setPickerSale( oldSetting );
            }
        
        // batches of 4 moves, with 4 being the lowest
        setAINumMovesToTest( 4 + newSetting * 4 );
        
        //setPlayerNumToSell( 0, newSale );
        
        if( doneButton->getPressed( inX, inY ) ) {
            waitingForDone = false;
            stateDone = true;
            
            // start the AI
            initOpponent( true );
            }
        }
    
            

    }



void SetAILevelState::stepState() {
    }



void SetAILevelState::drawState() {
    
    drawSprite( satelliteTopSpriteID, 
                0,0, white );
    drawSprite( satelliteBottomSpriteID, 
                0,satelliteBottomHalfOffset, white );
    
    startNewSpriteLayer();



        
    if( waitingForDone ) {
        doneButton->draw();
        

        // shadow of paper
        rgbaColor shadowColor = black;
        shadowColor.a = 64;
        
        drawSprite( aiLevelSheetSpriteID, 121, 52, shadowColor );

        startNewSpriteLayer();
        
        drawSprite( aiLevelSheetSpriteID, 123, 50, white );



        startNewSpriteLayer();

        drawSalePicker( 113, 58 );

        int aiLevel = ( getAINumMovesToTest() - 4 ) / 4 + 1;
        
        
        char *levelString = autoSprintf( "%s %d", 
                                         translate( "level" ), aiLevel );
        
        font16->drawString( levelString, 
                            145, 
                            52,
                            getGreenBarInkColor( 0, getMonthsLeft(), false ), 
                            alignLeft );
        
        delete [] levelString;
        }
    
    }





void SetAILevelState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_aiLevel" );
    statusSubMessage = translate( "phaseSubStatus_aiLevel" ); 
    
    waitingForDone = true;

    int numMovesToTest = getAINumMovesToTest();
    
    // picker adjusts in batches of 4, with 4 being the lowest
    setPickerSale( (numMovesToTest - 4) / 4 );
    }






// singleton
static SetAILevelState state;


GameState *setAILevelState = &state;

