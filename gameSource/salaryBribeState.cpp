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
static char sentMove = false;
static char gotMove = false;

// wait at least 2 seconds between wait and display
static int stepsSinceSentMove = 0;
static int minSteps = 30;


static char pickingSalary = false;
static char pickingBribe = false;


extern Button *doneButton;
extern const char *statusMessage;
extern const char *statusSubMessage;


extern char isWaitingOnOpponent;


static int backupLastBribingUnit[ numUnits ];



class SalaryBribeState : public GameState {
    public:

        SalaryBribeState() {
            mStateName = "SalaryBribeState";
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
            return "help_payAgents";
            }



        // destructor?
        //virtual ~GameState();
        
    };



static char isUnitPayable( int inUnit ) {
    if( inUnit < numPlayerUnits &&
        getUnitRegion( inUnit ) == 0 ) {
        // player unit that's home
        return true;
        }
    if( inUnit >= numPlayerUnits && 
        inUnit < 2 * numPlayerUnits ) {
        
        int unitRegion = getUnitRegion( inUnit );

        // is some player unit in this opponent's region?
        for( int i=0; i<numPlayerUnits; i++ ) {
            if( getUnitRegion( i ) == unitRegion ) {
                return true;
                }
            }
        }
    
    return false;
    }





// returns num payable
static int setPayableUnitsSelectable() {
    int numPayable = 0;

    // home player units
    int i;
    for( i=0; i<numPlayerUnits; i++ ) {
        if( getUnitRegion( i ) == 0 ) {
            setUnitSelectable( i, true );
            getUnit( i )->mShowSalaryPayment = true;
            
            numPayable++;
            }
        }
    // opponent units in shared regions
    for( i=0; i<numPlayerUnits; i++ ) {
        int oppUnit = i + numPlayerUnits;
        int oppRegion = getUnitRegion( oppUnit );
        
        for( int j=0; j<numPlayerUnits; j++ ) {
            if( getUnitRegion( j ) == oppRegion ) {
                setUnitSelectable( oppUnit, true );
                getUnit( oppUnit )->mShowBribePayment = true;
                
                numPayable++;
                }
            }
        }

    return numPayable;
    }




void SalaryBribeState::clickState( int inX, int inY ) {


    int activeUnit = getActiveUnit();
    Unit *u = NULL;

    if( activeUnit != -1 ) {
        u = getUnit( activeUnit );
        }
    

    if( pickingSalary || pickingBribe ) {
        
        if( activeUnit == -1 ) {
            printOut( "Error: Picking salary with no active unit!\n" );
            return;
            }        



        int oldBid = getPickerBid();
        char pickerClicked = clickBidPicker( inX, inY );
        int newBid = getPickerBid();
        

        if( newBid > oldBid &&
            getPlayerMoney( 0 ) == 0 ) {
            
            // out of money, stop bid increase
            newBid = oldBid;
            setPickerBid( newBid );
            }
        

        // update money
        addPlayerMoney( 0, - (newBid - oldBid) );
        
        if( pickingSalary ) {
            u->mLastSalaryPayment = newBid;
            }
        else {
            u->mLastBribePayment = newBid;
            if( newBid > 0 ) {
                // find bribing unit
                int oppRegion = getUnitRegion( activeUnit );
                for( int i=0; i<numPlayerUnits; i++ ) {
                    if( getUnitRegion( i ) == oppRegion ) {
                        u->mLastBribingUnit = i;
                        }
                    }
                }
            else {
                // restore to previous bribing unit (current bribe down to 0)
                u->mLastBribingUnit = backupLastBribingUnit[ activeUnit ];
                }
            }


        // check for a unit change INSTEAD of a picker-click
        // don't worry about if it's selectable, since none are
        // while we're picking a salary/bribe
        
        char unitChange = false;

        if( ! pickerClicked ) {

            int newChosenUnit = getChosenUnit( inX, inY, false );
            
            if( newChosenUnit != -1 && newChosenUnit != activeUnit &&
                isUnitPayable( newChosenUnit ) ) {
                unitChange = true;
                }
            }
        




        if( isBidDone() || unitChange ) {
            pickingSalary = false;
            pickingBribe = false;
            u->mShowSalaryPayment = false;
            u->mShowBribePayment = false;
            
            setPayableUnitsSelectable();

            statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                
            setActiveUnit( -1 );

            activeUnit = -1;
            }
    
        if( ! unitChange ) {
            return;
            }
        // else move on to next case below to handle change of picked unit

        }

    
    if( activeUnit == -1 ) {
        activeUnit = getChosenUnit( inX, inY );
                        
        if( activeUnit != -1 ) {
            setActiveUnit( activeUnit );
            u = getUnit( activeUnit );
            
            int oldBid;

            if( activeUnit < numPlayerUnits ) {
                pickingSalary = true;
                oldBid = u->mLastSalaryPayment;
                statusSubMessage = translate( "phaseSubStatus_pickSalary" );
                u->mShowSalaryPayment = true;
                }
            else {
                pickingBribe = true;
                oldBid = u->mLastBribePayment;

                statusSubMessage = 
                    translate( "phaseSubStatus_pickOpponentBribe" );
                u->mShowBribePayment = true;
                }
            

            setAllUnitsNotSelectable();

            setPickerBid( oldBid );
            }
        }

 
    if( activeUnit == -1 ) {
        if( doneButton->getPressed( inX, inY ) ) {
            setAllUnitsNotSelectable();
            
            
            
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );

            nextSubState();

            // send our move as a message

            // 2 chars per unit (both player and opponent)
            // salary or bribe, last bribing unit

            int messageLength = numPlayerUnits * 2 * 2;
            unsigned char message[ numPlayerUnits * 2 * 2 ];

            int i;
            for( i=0; i<numPlayerUnits*2; i++ ) {
                int index = i*2;
                
                Unit *u = getUnit( i );
                if( i < numPlayerUnits ) {
                    message[ index++ ] = 
                        (unsigned char)(u->mLastSalaryPayment);
                    }
                else {
                    message[ index++ ] = 
                        (unsigned char)(u->mLastBribePayment);
                    }
                message[ index++ ] =  (unsigned char)(u->mLastBribingUnit);
                }
            
            printOut( "Sending message: " );
            for( i=0; i<messageLength; i++ ) {
                printOut( "%d,", message[i] );
                }
            printOut( "\n" );
            
            sendOpponentMessage( message, (unsigned int)messageLength );
            sentMove = true;
            stepsSinceSentMove = 0;
            }
        
        }
    
    
    
    }



void SalaryBribeState::stepState() {
    
    stepUnits();

    stepsSinceSentMove++;
    
    if( sentMove && !gotMove && stepsSinceSentMove > minSteps ) {
        
        if( checkOpponentConnectionStatus() != 1 ) {
            connectionBroken = true;
            stateDone = true;
            return;
            }

        
        unsigned int messageLength;
        unsigned char *message = getOpponentMessage( &messageLength );
        //printOut( "trying to receive message\n" );

        isWaitingOnOpponent = true;
        
        if( message != NULL ) {
            isWaitingOnOpponent = false;
        
            // got move!

            gotMove = true;
            

            nextSubState();
            

            // unpack it
            if( messageLength != numPlayerUnits * 2 * 2 ) {
                printOut( "Bad message length from opponent\n" );
                stateDone = true;

                delete [] message;
                return;
                }
            

            int totalSalaryAndBribes = 0;
            
            int i;
            for( i=0; i<numPlayerUnits*2; i++ ) {
                

                int index = i * 2;
                // units described in opposite order

                // opp units first (opposite)
                
                if( i < numPlayerUnits ) {
                    // these are opp units
                    Unit *u = getUnit( i + numPlayerUnits );

                    // salary
                    u->mLastSalaryPayment = message[ index++ ];
                    totalSalaryAndBribes += u->mLastSalaryPayment;
                    
                    // ignore last bribing unit, since we already know it
                    }
                else {
                    // these bytes describe our units (from opponent's 
                    // point of view)
                    Unit *u = getUnit( i - numPlayerUnits );

                    u->mLastBribePayment = message[ index++ ];
                    totalSalaryAndBribes += u->mLastBribePayment;
                    
                    // might be -1
                    u->mLastBribingUnit = (char)message[ index++ ];
                    
                    
                    if( u->mLastBribingUnit >= 0 ) {
                        // adjust to describe opponent unit that is bribing us
                        u->mLastBribingUnit += numPlayerUnits;
                        }
                    
                    }
                }

            addPlayerMoney( 1, -totalSalaryAndBribes );
            

            
            for( i=0; i<numPlayerUnits * 2; i++ ) {
                getUnit(i)->mShowSalaryPayment = false;
                getUnit(i)->mShowBribePayment = false;
                }

            // now we have bids and bribes for both sides

            // apply them
            for( i=0; i<numPlayerUnits*2; i++ ) {
                Unit *u = getUnit(i);
                u->mTotalSalary += u->mLastSalaryPayment;
                u->mLastSalaryPayment = 0;

                u->mTotalBribe += u->mLastBribePayment;
                u->mLastBribePayment = 0;
                }
            

            // walk through again now that they're all applied
            for( i=0; i<numPlayerUnits*2; i++ ) {
                Unit *u = getUnit(i);

                char bribeKnown = false;
                
                // check status of bribing unit
                if( u->mLastBribingUnit >= 0 ) {
                    
                    Unit *bribingUnit = getUnit( u->mLastBribingUnit );
                    if( bribingUnit->mTotalBribe > 
                        bribingUnit->mTotalSalary ) {
                        
                        // bribe visible to unit's owner
                        u->mEnemyContactSinceBribeKnown = false;
                        bribeKnown = true;
                        
                        // we know this unit's current bribe
                        // remember it (bribe can only go up from here)
                        u->mMinKnownTotalBribe = u->mTotalBribe;
                        }
                    }
                
                if( !bribeKnown ) {
                    // is old knowledge still good?
                    // not if we're in the same region as an UNBRIBED enemy
                
                    if( u->mRegion > 1 ) {
                        // not home

                        for( int j=0; j<numPlayerUnits*2; j++ ) {
                            if( j != i ) {
                                if( getUnitRegion( j ) == u->mRegion ) {

                                    Unit *contactUnit = getUnit( j );

                                    if( contactUnit->mTotalBribe <= 
                                        contactUnit->mTotalSalary ) {
                                    
                                        // unit tainted by this contact
                                        u->mEnemyContactSinceBribeKnown = true;
                                        }
                                    } 
                                }
                            }
                        }                    
                    }
                


                }
            
            
            // opponent has had opportunity to spend money, so treat
            // it as an unknown from here on out
            setOpponentMoneyUnknown( true );
            

            stateDone = true;
            
            delete [] message;
            }        
        }
    
    }



void SalaryBribeState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    int activeUnit = getActiveUnit();

    if( activeUnit == -1 && ! sentMove ) {
        doneButton->draw();
        }
    

    if( pickingSalary ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bid with no active unit!\n" );
            return;
            }
    

        intPair bidPos = getUnitSalaryPosition( activeUnit );
        

        drawBidPicker( bidPos.x + 23, bidPos.y );
        drawBidBorder( bidPos.x, bidPos.y );
        }

    if( pickingBribe ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bribe with no active unit!\n" );
            return;
            }
    

        intPair bribePos = getUnitBribePosition( activeUnit );
        
        drawBidPicker( bribePos.x + 23, bribePos.y );
        drawBidBorder( bribePos.x, bribePos.y );
        }
    
    }





void SalaryBribeState::enterState() {
    stateDone = false;
    connectionBroken = false;
    
    sentMove = false;
    gotMove = false;
    
    setActiveUnit( -1 );
    showUnitMoves( false );
    
    // user needs to pick one
    int numPayable = setPayableUnitsSelectable();

    statusMessage = translate( "phaseStatus_payAgents" );

    if( numPayable > 0 && getPlayerMoney( 0 ) > 0 ) {
        
        statusSubMessage = translate( "phaseSubStatus_pickAgent" ); 
        }
    else {
        statusSubMessage = translate( "phaseSubStatus_payAgentCannot" ); 
        }
    
    for( int i=0; i<numUnits; i++ ) {
        backupLastBribingUnit[i] = getUnit(i)->mLastBribingUnit;
        }
    }






// singleton
static SalaryBribeState state;


GameState *salaryBribeState = &state;

