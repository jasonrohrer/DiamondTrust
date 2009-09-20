#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"


static int activeUnit = -1;
static char stateDone = false;

extern Button *doneButton;
extern char *statusMessage;



class MoveUnitsState : public GameState {
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





void MoveUnitsState::clickState( int inX, int inY ) {
    if( activeUnit == -1 ) {
        activeUnit = getChosenUnit( inX, inY );
                        
        if( activeUnit != -1 ) {
            setAllUnitsNotSelectable();
                    
            //int unitRegion = getUnitRegion( activeUnit );
            int unitDest = getUnitDestination( activeUnit );
                    
            // unit can move to any region that's not already
            // a destination of friendly units

            // can always move back home
            if( unitDest != 0 ) {
                setRegionSelectable( 0, true );
                }
            else {
                setRegionSelectable( 0, false );
                }
                    
                    
            // never move into region 1 (enemy home)

            for( int r=2; r<numMapRegions; r++ ) {
                char alreadyDest = false;
                        
                for( int u=0; u<numUnits-1 && !alreadyDest; u++ ) {
                    if( getUnitDestination( u ) == r ) {
                        alreadyDest = true;
                        }
                    }

                if( !alreadyDest ) {
                    setRegionSelectable( r, true );
                    }
                else {
                    setRegionSelectable( r, false );
                    }
                        
                }
            }
        }
    else {
        // unit already selected
                
        // region picked?
        int chosenRegion = getChosenRegion( inX, inY );
            
        if( chosenRegion != -1 ) {
            setUnitDestination( activeUnit, chosenRegion );
            activeUnit = -1;
            setPlayerUnitsSelectable( true );
            }
        }

 
    if( doneButton->getPressed( inX, inY ) ) {
        stateDone = true;
        }
    
    }



void MoveUnitsState::stepState() {
    
    stepUnits();
    }



void MoveUnitsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    
    doneButton->draw();
    }





void MoveUnitsState::enterState() {
    stateDone = false;
    activeUnit = -1;
    // user needs to pick one
    setPlayerUnitsSelectable( true );

    statusMessage = translate( "phaseStatus_move" );
    }






// singleton
static MoveUnitsState state;


GameState *moveUnitsState = &state;
