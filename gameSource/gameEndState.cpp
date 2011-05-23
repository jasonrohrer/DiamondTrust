#include "GameState.h"
#include "Button.h"
#include "units.h"
#include "map.h"
#include "ai/ai.h"
#include "common.h"
#include "gameStats.h"

//static int activeUnit = -1;
static char stateDone = false;

static char playingAgain = false;


extern Button *playAgainButton;

extern char isParent;
extern char networkOpponent;


extern char *statusMessage;
extern char *statusSubMessage;


// tells connected child whether we are playing again or not
static void sendNextGameFlag( char inPlayAgain ) {
    
    unsigned char message[1];
    message[0] = 1;

    if( ! inPlayAgain ) {
        message[0] = 0;
        
        // auto-close connection after this send
        sendLastMessage( message, 1 );
        }
    else {
        sendMessage( message, 1 );
        }
    }



class GameEndState : public GameState {
    public:
        
        GameEndState() {
            mStateName = "GameEndState";
            }
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        virtual char needsNextButton() {
            return false;
            }
        

        // back to go back to main menu
        virtual char canStateBeBackedOut() {
            // but only from parent
            return isParent;
            }
        

        virtual void backOutState();


        virtual int getParameter() {
            return playingAgain;
            }
        

        // destructor?
        //virtual ~GameState();
        
    };





static void resetToPlayAgain() {
    resetStats();
    resetMapDiamondCounts();
    resetUnits();
    // all units are home and empty
    
    // leave inspector alone

    resetAI();
    }




void GameEndState::clickState( int inX, int inY ) {
    
    if( isParent && playAgainButton->getPressed( inX, inY ) ) {

        playingAgain = true;
        
        resetToPlayAgain();
        
        if( networkOpponent ) {
            // playing again
            sendNextGameFlag( true );
            }

        
        stateDone = true;
        }

    }




void GameEndState::stepState() {
    // no stepping!

    if( !isParent ) {

        if( checkConnectionStatus() != 1 ) {
            
            // connection broken
            // not playing again
            
            playingAgain = false;
            
            stateDone = true;
            return;
            }
        
        // look for message telling us to play again

        unsigned int messageLength;
        unsigned char *message = getMessage( &messageLength );
                
        if( message != NULL ) {
                
            if( messageLength != 1 ) {
                printOut( "Bad message length from opponent\n" );
                playingAgain = false;

                stateDone = true;
                
                delete [] message;
                return;
                }
            
            if( message[0] == 1 ) {
                // playing again!
                playingAgain = true;
                }
            else {
                playingAgain = false;
                closeConnection();
                }

            delete [] message;
            
            // going back to menu OR playing again
            // reset for whatever next game we end up playing
            resetToPlayAgain();    

            stateDone = true;
            }
        
        }
    }



void GameEndState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    if( isParent  ) {
        playAgainButton->draw();
        }
    }





void GameEndState::enterState() {
    stateDone = false;
    playingAgain = false;
    
    int winner = 0;
    
    if( getPlayerDiamonds( 0 ) > getPlayerDiamonds( 1 ) ) {
        winner = 0;
        }
    else if( getPlayerDiamonds( 1 ) > getPlayerDiamonds( 0 ) ) {
        winner = 1;
        }
    else {
        // tie
        // break with money
        if( getPlayerMoney( 0 ) > getPlayerMoney( 1 ) ) {
            winner = 0;
            }
        else if( getPlayerMoney( 1 ) > getPlayerMoney( 0 ) ) {
            winner = 1;
            }
        else {
            // total tie
            winner = -1;
            }
        
        }
    
    switch( winner ) {
        case -1:
            statusMessage = translate( "phaseStatus_tie" );
            break;
        case 0:
            statusMessage = translate( "phaseStatus_youWin" );
            break;
        case 1:
            statusMessage = translate( "phaseStatus_opponentWins" );
            break;
        }
    
    statusSubMessage = "";
    
    showUnitMoves( false );
    
    showAllUnitMoves( false );
    }



void GameEndState::backOutState() {
    if( isParent ) {
        // going back to menu
        // reset for whatever next game we end up playing
        resetToPlayAgain();    


        playingAgain = false;
        
        if( networkOpponent ) {
            sendNextGameFlag( false );
            }
        }
        
    }






// singleton
static GameEndState state;


GameState *gameEndState = &state;
