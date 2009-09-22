#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;

extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;



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


static char pickingBid = false;
static char pickingBribe = false;




void MoveUnitsState::clickState( int inX, int inY ) {


    int activeUnit = getActiveUnit();

    if( pickingBid ) {
        
        if( activeUnit == -1 ) {
            printOut( "Error: Picking bid with no active unit!\n" );
            return;
            }
        
        
        int oldBid = getPickerBid();
        

        clickBidPicker( inX, inY );
        

        int newBid = getPickerBid();
        

        if( newBid > oldBid &&
            getPlayerMoney( 0 ) == 0 ) {
            
            // out of money, stop bid increase
            newBid = oldBid;
            setPickerBid( newBid );
            }
        
        // update money
        addPlayerMoney( 0, - (newBid - oldBid) );
        
        setUnitBid( activeUnit, newBid );

        if( isBidDone() ) {
            pickingBid = false;

            if( getUnitDestination( activeUnit ) == 
                getUnitRegion( numUnits - 1 ) ) {
                // inspector there

                pickingBribe = true;
                setUnitInspectorBribe( activeUnit, 0 );
                setPickerBid( 0 );
                showInspectorBribe( activeUnit, true );

                statusSubMessage = 
                    translate( "phaseSubStatus_pickInspectorBribe" );
                }
            else {
                
                setPlayerUnitsSelectable( true );

                statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                
                setActiveUnit( -1 );
                }
            }
    
        return;
        }

    if( pickingBribe ) {
        
        if( activeUnit == -1 ) {
            printOut( "Error: Picking bribe with no active unit!\n" );
            return;
            }
        
        
        
        int oldBid = getPickerBid();

        clickBidPicker( inX, inY );
        
        int newBid = getPickerBid();
        

        if( newBid > oldBid &&
            getPlayerMoney( 0 ) == 0 ) {
            
            // out of money, stop bid increase
            newBid = oldBid;
            setPickerBid( newBid );
            }
        
        // update money
        addPlayerMoney( 0, - (newBid - oldBid) );

        setUnitInspectorBribe( activeUnit, newBid );

        if( isBidDone() ) {
            setPlayerUnitsSelectable( true );

            statusSubMessage = translate( "phaseSubStatus_pickAgent" );

            pickingBribe = false;
            setActiveUnit( -1 );
            }
    
        return;
        }


    
    if( activeUnit == -1 ) {
        activeUnit = getChosenUnit( inX, inY );
                        
        if( activeUnit != -1 ) {
            setActiveUnit( activeUnit );
            
            statusSubMessage = translate( "phaseSubStatus_pickRegion" );


            setAllUnitsNotSelectable();

            //int unitDest = getUnitDestination( activeUnit );
            
            // clear any old move
            int unitRegion = getUnitRegion( activeUnit );
            setUnitDestination( activeUnit, unitRegion );
            
            int oldBid = getUnitBid( activeUnit );
            
            addPlayerMoney( 0, oldBid );
            setUnitBid( activeUnit, 0 );

            int oldBribe = getUnitInspectorBribe( activeUnit );
            
            addPlayerMoney( 0, oldBribe );
            setUnitInspectorBribe( activeUnit, 0 );

       
            // unit can move to any region that's not already
            // a destination of friendly units

            // can always move back home, if not already there
            if( unitRegion != 0 ) {                
                setRegionSelectable( 0, true );
                }
            else {
                setRegionSelectable( 0, false );
                }
                    
                    
            // never move into region 1 (enemy home)

            for( int r=2; r<numMapRegions; r++ ) {
                char alreadyDest = false;
                        
                for( int u=0; u<numUnits-1 && !alreadyDest; u++ ) {
                    int dest = getUnitDestination( u );
                    
                    if( dest == r ) {
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
            //setUnitBid( activeUnit, 0 );
            //setPickerBid( 0 );
            
            statusSubMessage = translate( "phaseSubStatus_pickBid" );

            

            setAllRegionsNotSelectable();
            

            if( chosenRegion != 0 ) {
                
                setPickerBid( 0 );
                pickingBid = true;
                showInspectorBribe( activeUnit, false );
                }
            else {
                setPlayerUnitsSelectable( true );
                setActiveUnit( -1 );

                statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                }
            
            }
        }

 
    if( activeUnit == -1 ) {
        if( doneButton->getPressed( inX, inY ) ) {
            stateDone = true;
            }
        }
    
    
    }



void MoveUnitsState::stepState() {
    
    stepUnits();
    }



void MoveUnitsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    int activeUnit = getActiveUnit();

    if( activeUnit == -1 ) {
        doneButton->draw();
        }
    

    if( pickingBid ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bid with no active unit!\n" );
            return;
            }
    

        intPair bidPos = getUnitBidPosition( activeUnit );
        

        drawBidPicker( bidPos.x - 18, bidPos.y );
        }

    if( pickingBribe ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bribe with no active unit!\n" );
            return;
            }
    

        intPair bribePos = getUnitInspectorBribePosition( activeUnit );
        

        drawBidPicker( bribePos.x - 18, bribePos.y );
        }
    
    }





void MoveUnitsState::enterState() {
    stateDone = false;
    
    setActiveUnit( -1 );
    
    // user needs to pick one
    setPlayerUnitsSelectable( true );

    statusMessage = translate( "phaseStatus_move" );
    statusSubMessage = translate( "phaseSubStatus_pickAgent" ); 
   
    }






// singleton
static MoveUnitsState state;


GameState *moveUnitsState = &state;

