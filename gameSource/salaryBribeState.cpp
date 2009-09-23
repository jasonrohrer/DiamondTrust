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
            
            int i;
            for( i=0; i<numPlayerUnits; i++ ) {
                getUnit(i)->mShowSalaryPayment = false;
                getUnit( i + numPlayerUnits )->mShowBribePayment = false;
                }
            
            stateDone = true;
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
            if( messageLength != numPlayerUnits * 3 ) {
                printOut( "Bad message length from opponent\n" );
                stateDone = true;

                delete [] message;
                return;
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

            addPlayerMoney( 1, -totalBidsAndBribes );
            

            statusSubMessage = 
                translate( "phaseSubStatus_moveExecute" );

            delete [] message;
            }        
        }
    
    }



void SalaryBribeState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    int activeUnit = getActiveUnit();

    if( activeUnit == -1 ) {
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
    
    // user needs to pick one
    setPayableUnitsSelectable();

    statusMessage = translate( "phaseStatus_payAgents" );
    statusSubMessage = translate( "phaseSubStatus_pickAgent" ); 
   
    }






// singleton
static SalaryBribeState state;


GameState *salaryBribeState = &state;

