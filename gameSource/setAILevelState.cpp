#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "salePicker.h"
#include "colors.h"
#include "ai/ai.h"

#include "minorGems/util/stringUtils.h"



static char stateDone = false;

extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;



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
        
        

        
        virtual ~SetAILevelState();
        
    protected:
        char *mMessage;
    };


SetAILevelState::SetAILevelState() 
        : mMessage( NULL ) {
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
        
        // batches of 5 seconds, with 5 being the lowest
        setAIThinkingTime( 5 + newSetting * 5 );
        
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
        
        drawSalePicker( 209, 58 );

        char *timeString = autoSprintf( "%d seconds", getAIThinkingTime() );

        // shadow
        rgbaColor shadowColor = black;
        shadowColor.a = 128;
        
        font16->drawString( timeString, 
                            198, 
                            53, shadowColor, alignRight );

        startNewSpriteLayer();
        
        font16->drawString( timeString, 
                            197, 
                            52, white, alignRight );
        
        delete [] timeString;
        }
    
    }





void SetAILevelState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_aiLevel" );
    statusSubMessage = translate( "phaseSubStatus_aiLevel" ); 
    
    waitingForDone = true;

    int thinkingTime = getAIThinkingTime();
    
    // picker adjusts in batches of 5, with 5 being the lowest
    setPickerSale( (thinkingTime - 5) / 5 );
    }






// singleton
static SetAILevelState state;


GameState *setAILevelState = &state;

