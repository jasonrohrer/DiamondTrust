#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;
static char sentMove = false;
static char gotMove = false;

static char pickingSalary = false;
static char pickingBribe = false;


extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;



class SalaryBribeState : public GameState {
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


static void setPayableUnitsSelectable() {
    // home player units
    int i;
    for( i=0; i<numPlayerUnits; i++ ) {
        if( getUnitRegion( i ) == 0 ) {
            setUnitSelectable( i, true );
            getUnit( i )->mShowSalaryPayment = true;
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
                }
            }
        }
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
            
            }

        if( isBidDone() ) {
            pickingSalary = false;
            pickingBribe = false;
            u->mShowSalaryPayment = false;
            u->mShowBribePayment = false;
            
            setPayableUnitsSelectable();

            statusSubMessage = translate( "phaseSubStatus_pickAgent" );
                
            setActiveUnit( -1 );
            }
    
        return;
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
            
            sendMessage( message, (unsigned int)messageLength );
            sentMove = true;
            }
        
        }
    
    
    
    }



void SalaryBribeState::stepState() {
    
    stepUnits();

    if( sentMove && !gotMove ) {
        
        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            stateDone = true;
            return;
            }

        
        unsigned int messageLength;
        unsigned char *message = getMessage( &messageLength );
        //printOut( "trying to receive message\n" );
        
        if( message != NULL ) {
            // got move!

            gotMove = true;
            

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
                    
                    u->mLastBribingUnit = message[ index++ ];
                    
                    // adjust to describe opponent unit that is bribing us
                    u->mLastBribingUnit += numPlayerUnits;
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
        

        drawBidPicker( bidPos.x + 18, bidPos.y );
        }

    if( pickingBribe ) {
        int activeUnit = getActiveUnit();

        if( activeUnit == -1 ) {
            printOut( "Error: Picking bribe with no active unit!\n" );
            return;
            }
    

        intPair bribePos = getUnitBribePosition( activeUnit );
        
        drawBidPicker( bribePos.x + 18, bribePos.y );
        }
    
    }





void SalaryBribeState::enterState() {
    stateDone = false;
    
    setActiveUnit( -1 );
    showUnitMoves( false );
    
    // user needs to pick one
    setPayableUnitsSelectable();

    statusMessage = translate( "phaseStatus_payAgents" );
    statusSubMessage = translate( "phaseSubStatus_pickAgent" ); 
   
    }






// singleton
static SalaryBribeState state;


GameState *salaryBribeState = &state;

