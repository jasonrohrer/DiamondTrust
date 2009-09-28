#include "GameState.h"
#include "units.h"
#include "map.h"
#include "common.h"
#include "flyingDiamonds.h"

//static int activeUnit = -1;
static char stateDone = false;

extern char *statusMessage;
extern char *statusSubMessage;



class ConfiscateState : public GameState {
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








void ConfiscateState::clickState( int inX, int inY ) {
    
    // no clicking!
    }



void ConfiscateState::stepState() {
    
    stepUnits();
    stepFlyingDiamonds();
    

    if( flyingDiamondAnimationDone() ) {
        
        // try starting another

        char found = false;
    

        for( int i=0; i<numPlayerUnits*2 && !found; i++ ) {
            
            Unit *u = getUnit( i );
            
            int unitRegion = getUnitRegion( i );
            
            if( u->mNumDiamondsHeld > 0 
                &&
                getUnitRegion( numUnits - 1 ) == unitRegion ) {
                
                            
                intPair unitPos = getUnitPositionInRegion( unitRegion, i );
                // center
                unitPos.y -= 8;
                
                intPair inspectorPos = getUnitPositionInRegion( unitRegion,
                                                                numUnits - 1 );
                // center
                inspectorPos.y -= 8;
                

                addFlyingDiamond( unitPos, inspectorPos );
                
                u->mNumDiamondsHeld --;
                
                found = true;
                }
            }


        if( !found ) {
            // last diamond flown
            
            stateDone = true;
            }
        }
    }



void ConfiscateState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    startNewSpriteLayer();

    drawFlyingDiamonds();
    }





void ConfiscateState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_confiscate" );    
    statusSubMessage = translate( "phaseSubStatus_confiscate" );    
    
    showUnitMoves( true );
    
    showAllUnitMoves( true );
    }






// singleton
static ConfiscateState state;


GameState *confiscateState = &state;
