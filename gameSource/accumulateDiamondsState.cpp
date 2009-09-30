#include "GameState.h"
#include "units.h"
#include "map.h"
#include "common.h"

//static int activeUnit = -1;
static char stateDone = false;

extern char *statusMessage;
extern char *statusSubMessage;



class AccumulateDiamondsState : public GameState {
    public:
        
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








void AccumulateDiamondsState::clickState( int inX, int inY ) {
    
    // no clicking!

    // avoid unused variable warnings
    inX = inY;
    }


static int stepsSinceDiamondTick = 0;
static int minDiamondSteps = 30;


void AccumulateDiamondsState::stepState() {

    stepsSinceDiamondTick++;
        
    if( stepsSinceDiamondTick > minDiamondSteps ) {
        stepsSinceDiamondTick = 0;
        char done =  accumulateDiamondsStep();
        
        
        if( done ) {
            stateDone = true;
            }
        }
    }



void AccumulateDiamondsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    }





void AccumulateDiamondsState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_accumulate" );    
    statusSubMessage = translate( "phaseSubStatus_accumulate" );    
    
    showUnitMoves( false );
    
    showAllUnitMoves( false );

    accumulateDiamondsStart();
    }






// singleton
static AccumulateDiamondsState state;


GameState *accumulateDiamondsState = &state;
