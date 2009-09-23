#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;
static char sentInitialMove = false;
static char gotInitialMove = false;
static char sentMove = false;
static char gotMove = false;


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




static void sendMoveMessage() {
    // send our move as a message

    // 3 chars per unit
    // dest, bid, bribe
    int messageLength = numPlayerUnits * 3;
    unsigned char message[ numPlayerUnits * 3 ];

    int i;
    for( i=0; i<numPlayerUnits; i++ ) {
        int index = i * 3;
                
        int dest = getUnitDestination( i );
        if( dest == 0 ) {
            // swap home between opponents
            dest = 1;
            }
        message[ index++ ] = (unsigned char)dest;
                
                
        message[ index++ ] = (unsigned char)getUnitBid( i );
        message[ index++ ] = (unsigned char)getUnitInspectorBribe( i );
        }
            
    printOut( "Sending message: " );
    for( i=0; i<messageLength; i++ ) {
        printOut( "%d,", message[i] );
        }
    printOut( "\n" );
            
    sendMessage( message, (unsigned int)messageLength );
    }


// returns total spent by opponent, or -1 if message not received
static int getMoveMessage() {
    unsigned int messageLength;
    unsigned char *message = getMessage( &messageLength );
    //printOut( "trying to receive message\n" );
        
    if( message != NULL ) {
        // got move!            

        // unpack it
        if( messageLength != numPlayerUnits * 3 ) {
            printOut( "Bad message length from opponent\n" );
            stateDone = true;

            delete [] message;
            return -1;
            }
            

        int totalBidsAndBribes = 0;
            
        int i;
        for( i=0; i<numPlayerUnits; i++ ) {
            int index = i * 3;
                
            int oppUnit = i + numPlayerUnits;
                
            setUnitDestination( oppUnit,
                                message[ index++ ] );
            
            int bid = message[ index++ ];
                
            setUnitBid( oppUnit, bid );
                
            totalBidsAndBribes += bid;
                
            int bribe = message[ index++ ];
                
            setUnitInspectorBribe( oppUnit, bribe );

            totalBidsAndBribes += bribe;
                
                
            if( getUnitDestination( oppUnit ) ==
                getUnitRegion( numUnits - 1 ) ) {
                    
                // moving in with inspector, show bribe
                showInspectorBribe( oppUnit, true );
                }
                
            }
        
        delete [] message;
        
        return totalBidsAndBribes;
        }

    return -1;
    }

        


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

 
    if( activeUnit == -1 && !sentInitialMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            setAllUnitsNotSelectable();
            
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );

            sendMoveMessage();
            
            sentInitialMove = true;
            }
        }
    
    
    }



void MoveUnitsState::stepState() {
    
    stepUnits();

    if( sentInitialMove && !gotMove ) {
        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            stateDone = true;
            return;
            }

        
        int totalBidsAndBribes = getMoveMessage();
        
        if( totalBidsAndBribes == -1 ) {
            // still waiting
            return;
            }
        else {
            
            statusMessage = translate( "phaseStatus_movePeek" );
            setMovePeeking( true );
            }        

        }
    

    if( sentMove && !gotMove ) {
        
        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            stateDone = true;
            return;
            }

        
        int totalBidsAndBribes = getMoveMessage();
        
        if( totalBidsAndBribes == -1 ) {
            // still waiting
            return;
            }
        else {
            
            addPlayerMoney( 1, -totalBidsAndBribes );
            

            statusSubMessage = 
                translate( "phaseSubStatus_moveExecute" );
            }        
        }
    
    }



void MoveUnitsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    int activeUnit = getActiveUnit();

    if( activeUnit == -1 && !sentMove ) {
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

