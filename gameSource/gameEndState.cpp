#include "GameState.h"
#include "Button.h"
#include "units.h"
#include "map.h"
#include "colors.h"
#include "ai/ai.h"
#include "common.h"
#include "gameStats.h"

//static int activeUnit = -1;
static char stateDone = false;

static char playingAgain = false;

static char donePressed = false;


extern Button *playAgainButton;
extern Button *doneButton;

extern char isHost;
extern char networkOpponent;


extern const char *statusMessage;
extern const char *statusSubMessage;


extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;
extern unsigned char satelliteFade;


extern char isWaitingOnOpponent;



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
            // and only after sat image faded back in
            return isHost && satelliteFade == 255;
            }
        

        virtual void backOutState();


        virtual int getParameter() {
            return playingAgain;
            }
        

        // destructor?
        //virtual ~GameState();
        
    };










void GameEndState::clickState( int inX, int inY ) {
    
    if( !donePressed ) {
        if( doneButton->getPressed( inX, inY ) ) {
            donePressed = true;
            
            // no explaination needed for parent because Play Again button
            // is clear
            if( !isHost ) {
                statusSubMessage = translate( "phaseSubStatus_playAgainWait" );
                isWaitingOnOpponent = true;
                }
            }
        }
    else if( isHost && playAgainButton->getPressed( inX, inY ) ) {

        playingAgain = true;
        
        if( networkOpponent ) {
            // playing again
            sendNextGameFlag( true );
            }

        
        stateDone = true;
        }

    }




void GameEndState::stepState() {
    
    if( !donePressed ) {
        // wait for done press on child
        return;
        }

    // fade satellite back in after Done
    if( satelliteFade < 255 ) {
        
        if( satelliteFade <= 251 ) {
            satelliteFade += 4;
            }
        else {
            satelliteFade = 255;
            }
        // don't allow other actions yet
        return;                              
        }

    if( !isHost ) {

        if( checkConnectionStatus() != 1 ) {
            
            // connection broken
            // not playing again
            isWaitingOnOpponent = false;
            
            playingAgain = false;
            
            stateDone = true;
            return;
            }
        
        // look for message telling us to play again

        unsigned int messageLength;
        unsigned char *message = getMessage( &messageLength );
                
        if( message != NULL ) {

            isWaitingOnOpponent = false;

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

            stateDone = true;
            }
        
        }
    }



void GameEndState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

     startNewSpriteLayer();
    
    if( satelliteFade > 0 ) {
        
        // draw fading satellite map on top
        rgbaColor satelliteColor = white;
        satelliteColor.a = satelliteFade;
        
        drawSprite( satelliteTopSpriteID, 
                    0,0, satelliteColor );
        drawSprite( satelliteBottomSpriteID, 
                    0,satelliteBottomHalfOffset, satelliteColor );
        }

    startNewSpriteLayer();

    if( !donePressed ) {
        doneButton->draw();
        }
    else if( isHost && satelliteFade == 255 && !stateDone ) {
        playAgainButton->draw();
        }
    }





void GameEndState::enterState() {
    stateDone = false;
    playingAgain = false;
    
    donePressed = false;

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
    if( isHost ) {
        playingAgain = false;
        
        if( networkOpponent ) {
            sendNextGameFlag( false );
            }
        }
        
    }






// singleton
static GameEndState state;


GameState *gameEndState = &state;
