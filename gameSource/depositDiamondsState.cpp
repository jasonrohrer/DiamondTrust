#include "GameState.h"
#include "units.h"
#include "map.h"
#include "common.h"
#include "flyingDiamonds.h"
#include "gameStats.h"

static char stateDone = false;

extern char *statusMessage;
extern char *statusSubMessage;



class DepositDiamondsState : public GameState {
    public:
        
        DepositDiamondsState() {
            mStateName = "DepositDiamondsState";
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








void DepositDiamondsState::clickState( int inX, int inY ) {
    
    // no clicking!

    // avoid unused variable warnings
    inX = inY;
    }



void DepositDiamondsState::stepState() {
    
    stepUnits();
    stepFlyingDiamonds();
    

    if( flyingDiamondAnimationDone() ) {
        
        // try starting another

        char found = false;
    

        for( int i=0; i<numPlayerUnits*2 && !found; i++ ) {
            
            Unit *u = getUnit( i );
            
            int unitRegion = getUnitRegion( i );
            int unitDiamonds = u->mNumDiamondsHeld;

            if( ( unitRegion == 0 || unitRegion == 1 ) 
                &&
                unitDiamonds > 0 ) {
                
                //printOut( "Unit diamonds = %d\n", unitDiamonds );
                
                found = true;
                
                intPair unitPos = getUnitPositionInRegion( unitRegion, i );
                // center
                unitPos.y -= 8;
                    
                addFlyingDiamond( 
                    unitPos,
                    getVaultPositionInRegion( unitRegion ) );
                    
                u->mNumDiamondsHeld --;
                
                // unit region = player number
                addPlayerDiamonds( unitRegion, 1 );
                }   
            }

        if( !found ) {
            // last diamond flown
            
            stateDone = true;
            }
        }
    }



void DepositDiamondsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    startNewSpriteLayer();

    drawFlyingDiamonds();
    }





void DepositDiamondsState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_depositDiamonds" );    
    statusSubMessage = translate( "phaseSubStatus_depositDiamonds" );    
        
    showUnitMoves( true );
    
    showAllUnitMoves( true );
    }






// singleton
static DepositDiamondsState state;


GameState *depositDiamondsState = &state;
