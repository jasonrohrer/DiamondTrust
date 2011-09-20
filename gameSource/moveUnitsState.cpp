#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"
#include "opponent.h"


//static int activeUnit = -1;
static char stateDone = false;
static char connectionBroken = false;
static char sentInitialMove = false;
static char gotInitialMove = false;
static int initialOpponentTotalSpent = 0;

static char sentMove = false;
static char gotMove = false;

static int stepsSinceSentMove = 0;
static int minSteps = 30;

static int stepsSinceExecute = 0;
static int minStepsSinceExecute = 60;

static const char *helpTransKey = "help_moveUnits";


extern Button *doneButton;
extern const char *statusMessage;
extern const char *statusSubMessage;


extern char isWaitingOnOpponent;



class MoveUnitsState : public GameState {
    public:

        MoveUnitsState() {
            mStateName = "MoveUnitsState";
            }
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        virtual char isConnectionBroken() {
            return connectionBroken;
            }


        virtual char canShowHelp() {
            return true;
            }
        
        virtual const char *getHelpTransKey() {
            return helpTransKey;
            }


        // destructor?
        //virtual ~GameState();
        
    };



static int getMoveCost( int inStartRegion, int inEndRegion ) {
    if( inStartRegion == inEndRegion ) {
        return 0;
        }
    if( inStartRegion == 0 || inEndRegion == 0 || 
        inStartRegion == 1 || inEndRegion == 1 ) {
        // moving to/from home region
        return 2;
        }
    // moving between producing regions
    return 1;
    }


static char shouldDrawDoneButton() {
    int activeUnit = getActiveUnit();

    if( activeUnit == -1 && !sentMove
        &&
        !( sentInitialMove && !gotInitialMove ) ) {
        return true;
        }

    return false;
    }




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
            
    sendOpponentMessage( message, (unsigned int)messageLength );
    }


// returns total spent by opponent, or -1 if message not received
static int getMoveMessage() {
    unsigned int messageLength;
    unsigned char *message = getOpponentMessage( &messageLength );
    //printOut( "trying to receive message\n" );
    
    isWaitingOnOpponent = true;
    
    if( message != NULL ) {

        // unpack it
        if( messageLength != numPlayerUnits * 3 ) {
            printOut( "Bad message length from opponent\n" );
            
            closeOpponentConnection();
            connectionBroken = true;

            delete [] message;
            return -1;
            }

        isWaitingOnOpponent = false;
        
        // got move!            
            

        int totalBidsAndBribes = 0;
            
        int i;
        for( i=0; i<numPlayerUnits; i++ ) {
            int index = i * 3;
                
            int oppUnit = i + numPlayerUnits;
                
            setUnitDestination( oppUnit,
                                message[ index++ ] );

            totalBidsAndBribes += getMoveCost( getUnitRegion( oppUnit ),
                                               getUnitDestination( oppUnit ) );
            
            
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

    if( !
        ( shouldDrawDoneButton() 
          ||
          pickingBid
          ||
          pickingBribe 
          ||
          activeUnit != -1 ) ) {
    
        // state is not clickable now, ignore click
        
        // since we're also looking for clicks on the bid and bribe markers 
        // below, we can't simply count on the units not being selectable
        // while we're waiting for the opponent to move, as we do in the
        // other states.

        return;
        }
    



    


    char considerSwitchingActive = true;
    

    if( ( pickingBribe || pickingBid )
        &&
        bidPickerHit( inX, inY ) ) {
        
        // give picker widget priority, because sometimes it overlaps
        // with other bid/bribe markers
        considerSwitchingActive = false;
        }
    

    


    int hitBidMarkerUnit = getChosenBidMarker( inX, inY );
    int hitBribeMarkerUnit = getChosenInspectorBribeMarker( inX, inY );
    

    if( hitBidMarkerUnit >= numPlayerUnits ) {
        // an opponent unit's bid marker hit---never let someone select it!
        hitBidMarkerUnit = -1;
        }
    if( hitBribeMarkerUnit >= numPlayerUnits ) {
        // an opponent unit's inspector bribe marker hit---never let someone 
        // select it!
        hitBribeMarkerUnit = -1;
        }

    // also consider switching active unit (when none might be selectable
    int newActiveUnit = getChosenUnit( inX, inY, false );

    // avoid passing first click to bid picker after a unit change,
    // because the picker only moves after it is next redrawn, and it may
    // register faulty clicks from it's old position
    char unitChange = false;

    if( considerSwitchingActive &&
        hitBidMarkerUnit != -1 && 
        ( hitBidMarkerUnit != activeUnit || 
          ( hitBidMarkerUnit == activeUnit && ! pickingBid ) ) ) {
        
        // instantly switch to adjusting bid for this unit
        

        if( hitBidMarkerUnit != activeUnit ) {
            // make sure we don't abandon active unit half-way through
            // picking bid/bribe combo in inspector's region
            // (because bribe is invisible while picking bribe to avoid
            //  confusion---don't leave it invisible upon abandoning!)
            if( activeUnit != -1 &&
                getUnitDestination( activeUnit ) == 
                getUnitRegion( numUnits - 1 ) ) {
                
                showInspectorBribe( activeUnit, true );
                }
            }
        
            
        setActiveUnit( hitBidMarkerUnit );

        statusSubMessage = 
            translate( "phaseSubStatus_pickBid" );

        
        activeUnit = getActiveUnit();

        // keep old bid for adjustment
        int oldBid = getUnitBid( activeUnit );
        setPickerBid( oldBid );
        
        showInspectorBribe( activeUnit, false );
        
        pickingBid = true;
        pickingBribe = false;

        unitChange = true;

        setAllRegionsNotSelectable();
        setAllUnitsNotSelectable();
        }
    else if( considerSwitchingActive &&
             hitBribeMarkerUnit != -1 && 
             ( hitBribeMarkerUnit != activeUnit || 
               ( hitBribeMarkerUnit == activeUnit && ! pickingBribe ) ) ) {
        
        // instantly switch to adjusting bribe for this unit
        
            
        setActiveUnit( hitBribeMarkerUnit );
        
        statusSubMessage = 
            translate( "phaseSubStatus_pickInspectorBribe" );
        
        activeUnit = getActiveUnit();

        // keep old bribe for adjustment
        int oldBribe = getUnitInspectorBribe( activeUnit );
        setPickerBid( oldBribe );
        
        showInspectorBribe( activeUnit, true );
        
        pickingBid = false;
        pickingBribe = true;

        unitChange = true;

        setAllRegionsNotSelectable();
        setAllUnitsNotSelectable();
        }    
    else if( activeUnit != -1 &&
             considerSwitchingActive &&
             newActiveUnit != -1 &&
             newActiveUnit < numPlayerUnits &&
             newActiveUnit != activeUnit &&
             // not at home, where more than one unit can land
             // which makes trying to click home as a destination, when
             // other units are there, confusing if those units suddenly
             // become selected
             // BUT, if we're picking a bid or a bribe already, then
             // we're not picking a destination, so it's okay to switch
             // to a home-region unit
             ( pickingBid || 
               pickingBribe ||
               getUnitRegion( newActiveUnit ) != 0 ) ) {
        
        
        // instantly switch to moving this unit
        
        

        if( newActiveUnit != activeUnit ) {


            // we KNOW that active unit isn't -1 here, because we
            // checked it above

            // make sure we don't abandon active unit half-way through
            // picking bid/bribe combo in inspector's region
            // (because bribe is invisible while picking bribe to avoid
            //  confusion---don't leave it invisible upon abandoning!)
            if( getUnitDestination( activeUnit ) == 
                getUnitRegion( numUnits - 1 ) ) {
                
                showInspectorBribe( activeUnit, true );
                }
            }
        

        // can handle setup for a new active unit in case below
        // by simply turning off active unit here
        setActiveUnit( -1 );
        activeUnit = -1;
        
        setPlayerUnitsSelectable( true );
        //showInspectorBribe( activeUnit, false );
        
        pickingBid = false;
        pickingBribe = false;

        setAllRegionsNotSelectable();
        }
    








    if( !unitChange && pickingBid ) {
        
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
                //setUnitInspectorBribe( activeUnit, 0 );
                setPickerBid( getUnitInspectorBribe( activeUnit ) );
                showInspectorBribe( activeUnit, true );

                statusSubMessage = 
                    translate( "phaseSubStatus_pickInspectorBribe" );
                }
            else {
                
                setPlayerUnitsSelectable( true );

                if( !sentInitialMove ) {
                    statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                    }
                else {
                    statusSubMessage = 
                        translate( "phaseSubStatus_pickAgentAdjust" );
                    }
                    
                setActiveUnit( -1 );
                }
            }
    
        return;
        }

    if( !unitChange && pickingBribe ) {
        
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


            if( !sentInitialMove ) {
                statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                }
            else {
                statusSubMessage = 
                    translate( "phaseSubStatus_pickAgentAdjust" );
                }

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

       
            // unit can move to any region that's not already
            // a destination of friendly units
            
            char affordable[ numMapRegions ];
            

            // take current move cost, which will be refunded, into
            // account when figuring out where player can afford to switch
            // move to
            int currentRegion = getUnitRegion( activeUnit );
            int currentDestination = getUnitDestination( activeUnit );

            int currentBid = getUnitBid( activeUnit );
            int currentBribe = getUnitInspectorBribe( activeUnit );
            
            int currentTotalMoveCost = 
                getMoveCost( currentRegion, currentDestination )
                + currentBid + currentBribe;
            


            int r;
            for( r=0; r<numMapRegions; r++ ) {
                
                if( r == 1 ) {
                    // forbidden
                    affordable[r] = false;
                    }
                else {
                    if( getMoveCost( currentRegion, r ) 
                        <= getPlayerMoney( 0 ) + currentTotalMoveCost ) {
                        
                        // we have enough money to move there, if the cost
                        // of our currently-pending move is refunded
                        affordable[r] = true;
                        }
                    else {
                        affordable[r] = false;
                        }
                    }                    
                }
            
                
                

            setRegionSelectable( 0, affordable[0] );
                                
                    
            // never move into region 1 (enemy home)
            
            for( int r=2; r<numMapRegions; r++ ) {
                char alreadyDest = false;
                        
                for( int u=0; u<numPlayerUnits && !alreadyDest; u++ ) {
                    int dest = getUnitDestination( u );
                    
                    // allow unit to "move into" its current non-home
                    // region to re-specify its bid there
                    if( u != activeUnit ) {
                        
                        if( dest == r ) {
                            alreadyDest = true;
                            }
                        }
                    }

                if( !alreadyDest ) {
                    setRegionSelectable( r, affordable[r] );
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
            int currentRegion = getUnitRegion( activeUnit );
            
            // different dest chose?
            int oldDest = getUnitDestination( activeUnit );
            
            if( oldDest != chosenRegion ) {
                // clear old bid info
                int oldBid = getUnitBid( activeUnit );
            
                addPlayerMoney( 0, oldBid );
                setUnitBid( activeUnit, 0 );

                int oldBribe = getUnitInspectorBribe( activeUnit );
            
                addPlayerMoney( 0, oldBribe );
                setUnitInspectorBribe( activeUnit, 0 );
            
                setUnitDestination( activeUnit, chosenRegion );
                setPickerBid( 0 );


                int oldMoveCost = getMoveCost( currentRegion, oldDest );
                addPlayerMoney( 0, oldMoveCost );
                
                int newMoveCost = getMoveCost( currentRegion, chosenRegion );
                addPlayerMoney( 0, -newMoveCost );

                getUnit( activeUnit )->mTripCost = newMoveCost;
                }
            else {
                // same region
                
                // keep old bid for adjustment
                int oldBid = getUnitBid( activeUnit );
                setPickerBid( oldBid );
                }
            
            
            //setUnitBid( activeUnit, 0 );
            //setPickerBid( 0 );
            
            statusSubMessage = translate( "phaseSubStatus_pickBid" );

            

            setAllRegionsNotSelectable();
            

            if( chosenRegion != 0 ) {
                pickingBid = true;
                showInspectorBribe( activeUnit, false );
                }
            else {
                setPlayerUnitsSelectable( true );
                setActiveUnit( -1 );
                
                if( !sentInitialMove ) {
                    statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                    }
                else {
                    statusSubMessage = 
                        translate( "phaseSubStatus_pickAgentAdjust" );
                    }
                
                }
            
            }
        }

 
    if( activeUnit == -1 && !sentInitialMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            setAllUnitsNotSelectable();

            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );

            sendMoveMessage();
            stepsSinceSentMove = 0;
            
            sentInitialMove = true;

            nextSubState();
            }
        }
    else if( activeUnit == -1 && !sentMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            setAllUnitsNotSelectable();
            
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );
            
            sendMoveMessage();
            stepsSinceSentMove = 0;

            sentMove = true;

            nextSubState();
            }
        }
    
    
    
    }



void MoveUnitsState::stepState() {
    
    stepUnits();
    stepsSinceSentMove ++;
    stepsSinceExecute ++;
    
    if( sentInitialMove && !gotInitialMove && stepsSinceSentMove > minSteps ) {
        if( checkOpponentConnectionStatus() != 1 ) {
            connectionBroken = true;
            stateDone = true;
            return;
            }

        
        int totalBidsAndBribes = getMoveMessage();
        
        if( totalBidsAndBribes == -1 ) {
            // still waiting
            return;
            }
        else {
            gotInitialMove = true;

            // subtract this from opponent money total so that we
            // see their true money balance if that info has been revealed 
            // to us
            // BUT, also remember this value, because we'll have to add
            // it back in later after the final move arrives, because
            // they amount they spend might change after peeking.
            initialOpponentTotalSpent = totalBidsAndBribes;
            
            addPlayerMoney( 1, -initialOpponentTotalSpent );
            

            // advance sub-state whether or not we actually can peek here
            // (music transition will hint that opponent is peeking)
            nextSubState();

            if( isAnyOpponentBribed() || isAnyPlayerUnitKnownBribed() ) {
                // show opponent moves to player and let player adjust

                statusMessage = translate( "phaseStatus_movePeek" );
                statusSubMessage = 
                    translate( "phaseSubStatus_pickAgentAdjust" );
                setMovePeeking( true );
                
                setPlayerUnitsSelectable( true );
                
                helpTransKey = "help_moveUnitsPeek";
                }
            else {
                // no new info, no need to adjust!

                // send final move to opponent right away
                setAllUnitsNotSelectable();
                
                statusSubMessage = 
                    translate( "phaseSubStatus_waitingOpponent" );
            
                sendMoveMessage();
                
                sentMove = true;
                }                
            }        

        }
    

    if( sentMove && !gotMove && stepsSinceSentMove > minSteps ) {
        
        if( checkOpponentConnectionStatus() != 1 ) {
            connectionBroken = true;
            stateDone = true;
            return;
            }

        
        int totalBidsAndBribes = getMoveMessage();
        
        if( totalBidsAndBribes == -1 ) {
            // still waiting
            return;
            }
        else {
            gotMove = true;
            
            // amount spent may have changed
            // add back in old, tentative spend total first
            addPlayerMoney( 1, initialOpponentTotalSpent );
            
            // now subtract the final spend total
            addPlayerMoney( 1, -totalBidsAndBribes );
            

            statusSubMessage = 
                translate( "phaseSubStatus_moveExecute" );
            
            showAllUnitMoves( true );
            executeUnitMoves();
            
            setMovePeeking( false );
            
            stepsSinceExecute = 0;

            nextSubState();
            }        
        }

    if( sentMove && gotMove ) {
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
    
    
    }



void MoveUnitsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    if( shouldDrawDoneButton() ) {
        doneButton->draw();
        }
    

    if( pickingBid ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bid with no active unit!\n" );
            return;
            }
    

        intPair bidPos = getUnitBidPosition( activeUnit );
        

        drawBidPicker( bidPos.x - 23, bidPos.y );
        drawBidBorder( bidPos.x, bidPos.y );
        }

    if( pickingBribe ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bribe with no active unit!\n" );
            return;
            }
    

        intPair bribePos = getUnitInspectorBribePosition( activeUnit );
        

        drawBidPicker( bribePos.x - 23, bribePos.y );
        drawBidBorder( bribePos.x, bribePos.y );
        }
    
    }





void MoveUnitsState::enterState() {
    stateDone = false;
    connectionBroken = false;
    
    sentInitialMove = false;
    gotInitialMove = false;
    sentMove = false;
    gotMove = false;
    
    setActiveUnit( -1 );
    showUnitMoves( true );

    pickingBid = false;
    pickingBribe = false;

    setMovePeeking( false );

    helpTransKey = "help_moveUnits";

    
    for( int i=0; i<numPlayerUnits * 2; i++ ) {
        setUnitBid( i, 0 );
        setUnitInspectorBribe( i, 0 );

        // turn off inspector bribe tags for any unit not in
        // inspector's region (might be on, left over from last move)
        int inspRegion = getUnitRegion( numUnits - 1 );
        
        for( int i=0; i<numPlayerUnits*2; i++ ) {
            if( getUnitRegion( i ) != inspRegion ) {
                
                showInspectorBribe( i, false );
                }
            else {
                showInspectorBribe( i, true );
                }
            
            }
            
        }
    


    // user needs to pick one
    setPlayerUnitsSelectable( true );

    statusMessage = translate( "phaseStatus_move" );
    statusSubMessage = translate( "phaseSubStatus_pickAgent" ); 
   
    }






// singleton
static MoveUnitsState state;


GameState *moveUnitsState = &state;

