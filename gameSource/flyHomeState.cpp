#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;

static int stepsSinceExecute = 0;
static int minStepsSinceExecute = 60;


extern char *statusMessage;
extern char *statusSubMessage;



class FlyHomeState : public GameState {
    public:

        FlyHomeState() {
            mStateName = "FlyHomeState";
            }
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        

        // destructor?
        //virtual ~GameState();
        
    };







void FlyHomeState::clickState( int inX, int inY ) {
    
    // no clicking
    
    // avoid unused variable warnings
    inX = inY;
    }



void FlyHomeState::stepState() {
    
    stepUnits();
    stepsSinceExecute ++;
    
    if( unitAnimationsDone() 
        && 
        stepsSinceExecute > minStepsSinceExecute ) {
        
        // use minimum steps to avoid flicker if 
        // no one is moving or winning
        
        
        for( int i=0; i<numPlayerUnits*2; i++ ) {
            // reset trip costs
            getUnit( i )->mTripCost = 0;
            }
            
        stateDone = true;
        }
    }



void FlyHomeState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    }





void FlyHomeState::enterState() {
    stateDone = false;
    
    setActiveUnit( -1 );
    showUnitMoves( true );


    showAllUnitMoves( true );
    setMovePeeking( false );
    
    for( int i=0; i<numPlayerUnits * 2; i++ ) {
        setUnitDestination( i, 0 );
        }
    for( int i=numPlayerUnits; i<numPlayerUnits * 2; i++ ) {
        setUnitDestination( i, 1 );
        }
    
    
    
    statusMessage = translate( "phaseStatus_flyHome" );
    statusSubMessage = translate( "phaseSubStatus_flyHome" ); 
   
    executeUnitMoves();
    stepsSinceExecute = 0;
    }






// singleton
static FlyHomeState state;


GameState *flyHomeState = &state;

