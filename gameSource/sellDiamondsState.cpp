#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "salePicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;
static char sentInitialMove = false;
static char gotInitialMove = false;
static char sentMove = false;
static char gotMove = false;

static char stepsSinceSentMove = 0;
static char minSteps = 30;


extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;



class SellDiamondsState : public GameState {
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

    // 1 char
    // number sold
    int messageLength = 1;
    unsigned char message[ 1 ];

    message[0] = (unsigned char)getPickerSale();
                
    printOut( "Sending message: " );
    for( int i=0; i<messageLength; i++ ) {
        printOut( "%d,", message[i] );
        }
    printOut( "\n" );
            
    sendMessage( message, (unsigned int)messageLength );
    }


// returns total sold by opponent, or -1 if message not received
static int getMoveMessage() {
    unsigned int messageLength;
    unsigned char *message = getMessage( &messageLength );
    //printOut( "trying to receive message\n" );
        
    if( message != NULL ) {
        // got move!            

        // unpack it
        if( messageLength != 1 ) {
            printOut( "Bad message length from opponent\n" );
            stateDone = true;

            delete [] message;
            return -1;
            }
            
        int returnVal = (int)message[0];
        
        setPlayerNumToSell( 1, returnVal );
        

        delete [] message;
        
        return returnVal;
        }

    return -1;
    }

        


static char pickingSale = false;




void SellDiamondsState::clickState( int inX, int inY ) {


    if( pickingSale ) {
        
        int oldSale = getPickerSale();
        

        clickSalePicker( inX, inY );
        

        int newSale = getPickerSale();
        
        if( newSale > getPlayerDiamonds( 0 ) ) {
            // too many!  stop
            newSale = oldSale;
            setPickerSale( oldSale );
            }
        

        setPlayerNumToSell( 0, newSale );
        }
    
 
    if( !sentInitialMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            pickingSale = false;
            
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );

            sendMoveMessage();
            stepsSinceSentMove = 0;
            
            sentInitialMove = true;
            }
        }
    else if( ( sentInitialMove && gotInitialMove ) && !sentMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );
            
            sendMoveMessage();
            stepsSinceSentMove = 0;

            sentMove = true;
            }
        }
    
    
    
    }



static int playerEarnings[2] = {0,0};
static int numSold[2] = {0,0};

static int stepsSinceEarningTick = 0;
static int minEarningsSteps = 10;


void SellDiamondsState::stepState() {
    
    stepUnits();
    stepsSinceSentMove ++;
    
    if( sentInitialMove && !gotInitialMove && stepsSinceSentMove > minSteps ) {
        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            stateDone = true;
            return;
            }

        
        int opponentSale = getMoveMessage();
        
        if( opponentSale == -1 ) {
            // still waiting
            return;
            }
        else {
            gotInitialMove = true;
            

            if( isOpponentHomeBribed() || isPlayerHomeKnownBribed() ) {
                // show opponent sale to player and let player adjust

                statusMessage = translate( "phaseStatus_sellPeek" );
                statusSubMessage = 
                    translate( "phaseSubStatus_sellDiamondsAdjust" );
                
                peekSale();
                
                pickingSale = true;
                }
            else {
                // no new info, no need to adjust!

                // send final move to opponent right away
                statusSubMessage = 
                    translate( "phaseSubStatus_waitingOpponent" );
            
                sendMoveMessage();
                
                sentMove = true;
                }                
            }        

        }
    

    if( gotInitialMove && sentMove && !gotMove && 
        stepsSinceSentMove > minSteps ) {
        
        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            stateDone = true;
            return;
            }

        
        int opponentSale = getMoveMessage();
        
        if( opponentSale == -1 ) {
            // still waiting
            return;
            }
        else {
            gotMove = true;
            
                        

            statusSubMessage = 
                translate( "phaseSubStatus_moveExecute" );
            
            finishSale();
            
            // tally earnings
            for( int i=0; i<2; i++ ) {
                playerEarnings[i] = getPlayerEarnings(i);
                numSold[i] = getNumSold(i);
                }
            
            stepsSinceEarningTick = 0;
            }        
        }

    if( sentMove && gotMove ) {
        
        stepsSinceEarningTick++;
        
        if( stepsSinceEarningTick > minEarningsSteps ) {
            
            char changedSome = false;
            
            // first tick diamonds
        
            for( int i=0; i<2; i++ ) {
                if( numSold[i] > 0 ) {
                    // more left to tally
                    addPlayerDiamonds( i, -1 );
                    numSold[i]--;
                    
                    changedSome = true;
                    stepsSinceEarningTick = 0;
                    }
                }


            if( !changedSome ) {
                for( int i=0; i<2; i++ ) {
                    if( playerEarnings[i] > 0 ) {
                        // more left to tally
                        addPlayerMoney( i, 1 );
                        playerEarnings[i]--;
                        
                        changedSome = true;
                        stepsSinceEarningTick = 0;
                        }
                    }
                }
            
        

            if( !changedSome ) {
                showSale( false );

                stateDone = true;
                }
            }
        
        }
    
    
    }



void SellDiamondsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    if( !sentMove
        &&
        !( sentInitialMove && !gotInitialMove ) ) {
        doneButton->draw();
        }
    

    if( pickingSale ) {
        drawSalePicker( 24, 137 );
        }    
    }





void SellDiamondsState::enterState() {
    stateDone = false;
    sentInitialMove = false;
    gotInitialMove = false;
    sentMove = false;
    gotMove = false;

    pickingSale = true;
    setPickerSale( 0 );
    
    showSale( true );
    
    setActiveUnit( -1 );
    showUnitMoves( false );
    showAllUnitMoves( false );
    
    statusMessage = translate( "phaseStatus_sellDiamonds" );
    statusSubMessage = translate( "phaseSubStatus_sellDiamonds" );

    if( getPlayerDiamonds( 0 ) == 0 ) {
        // we've got none to sell
        // don't trouble user with non-choice
        
        pickingSale = false;
        
        // wait for player to hit done
        
        statusSubMessage = translate( "phaseSubStatus_sellDiamondsNone" );
        }
    }






// singleton
static SellDiamondsState state;


GameState *sellDiamondsState = &state;

