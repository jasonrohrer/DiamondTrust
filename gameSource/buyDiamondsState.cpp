#include "GameState.h"
#include "units.h"
#include "map.h"
#include "common.h"
#include "flyingDiamonds.h"

//static int activeUnit = -1;
static char stateDone = false;

extern const char *statusMessage;
extern const char *statusSubMessage;



class BuyDiamondsState : public GameState {
    public:
        
        BuyDiamondsState() {
            mStateName = "BuyDiamondsState";
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








void BuyDiamondsState::clickState( int inX, int inY ) {
    
    // no clicking!

    // avoid unused variable warnings
    inX = inY;
    }



void BuyDiamondsState::stepState() {
    
    stepUnits();
    stepFlyingDiamonds();
    

    if( flyingDiamondAnimationDone() ) {
        
        // try starting another

        char found = false;
    

        for( int i=0; i<numPlayerUnits*2 && !found; i++ ) {
            
            Unit *u = getUnit( i );
            
            int unitRegion = getUnitRegion( i );
            int unitBid = getUnitBid( i );
            char highest = true;
            
            if( unitBid > 0 && getDiamondsInRegion( unitRegion ) 
                // inspector blocks sale
                &&
                getUnitRegion( numUnits - 1 ) != unitRegion ) {
                
                // look for other unit in this region
                for( int j=0; j<numPlayerUnits*2; j++ ) {
                    if( j != i ) {
                        if( getUnitRegion( j ) == unitRegion ) {
                            if( getUnitBid( j ) >= unitBid ) {
                                highest = false;
                                }
                            }
                        }
                    }
                
                if( highest ) {
                    // winner
                    intPair unitPos = getUnitPositionInRegion( unitRegion, i );
                    // center
                    unitPos.y -= 8;
                    
                    addFlyingDiamond( 
                        getDiamondPositionInRegion( unitRegion ),
                        unitPos );
                    
                    decrementDiamonds( unitRegion );
                    u->mNumDiamondsHeld ++;
                    
                    found = true;
                    }                
                }
            }

        if( !found ) {
            // last diamond flown
            
            stateDone = true;
            }
        }
    }



void BuyDiamondsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    startNewSpriteLayer();

    drawFlyingDiamonds();
    }





void BuyDiamondsState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_buyDiamonds" );    
    statusSubMessage = translate( "phaseSubStatus_buyDiamonds" );    
    
    showUnitMoves( true );
    
    showAllUnitMoves( true );
    }






// singleton
static BuyDiamondsState state;


GameState *buyDiamondsState = &state;
