#include "GameState.h"
#include "Button.h"
#include "units.h"
#include "map.h"
#include "colors.h"
#include "ai/ai.h"
#include "common.h"
#include "gameStats.h"
#include "music.h"

//static int activeUnit = -1;
static char stateDone = false;

static char playingAgain = false;

static char donePressed = false;

static char checkedOnceForPlayAgainMessage = false;


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
            // can back out either from parent or child

            // IF connection has not broken already
            if( checkOpponentConnectionStatus() == 1 ) {
                // but only after sat image faded back in
                if( satelliteFade == 255 ) {
                
                    // don't show cancel button yet, if we haven't
                    // checked for a PlayAgain message yet, because
                    // the message might already be waiting there for us,
                    // which causes the cancel button to flicker for a single
                    // frame
                    if( !isHost && !checkedOnceForPlayAgainMessage ) {
                        return false;
                        }
                    else {
                        return true;
                        }
                    }
                }

            return false;
            }
        

        // override default behavior just for gameEndState
        virtual char isGameFullyEnded() {
            return satelliteFade == 255;
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

            // back to limiting total number of tracks (get ready for player
            // to pick hosting Single-Card Play, which causes a CPU hicup
            limitTotalMusicTracks( true );

            // no explaination needed for parent because Play Again button
            // is clear
            if( !isHost ) {
                statusSubMessage = translate( "phaseSubStatus_playAgainWait" );
                isWaitingOnOpponent = true;
                }
            }
        }
    else if( satelliteFade == 255 && 
             checkOpponentConnectionStatus() == 1 &&
             isHost && playAgainButton->getPressed( inX, inY ) ) {

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

        checkedOnceForPlayAgainMessage = true;
                
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
    else if( isHost && satelliteFade == 255 && !stateDone &&
             checkOpponentConnectionStatus() == 1 ) {
        
        // Don't draw playAgain button at all if connection is already broken
        // this prevents it from being drawn for a single frame,
        // which causes an annoying flicker (in the case where the connection
        // broke during game-end proceedings but hasn't been reported yet)

        playAgainButton->draw();
        }
    }





void GameEndState::enterState() {
    stateDone = false;
    playingAgain = false;
    
    donePressed = false;

    checkedOnceForPlayAgainMessage = false;

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
    playingAgain = false;
        
    if( isHost ) {
        
        if( networkOpponent ) {
            sendNextGameFlag( false );
            }
        }
    else {
        // child ?
        if( networkOpponent ) {
            isWaitingOnOpponent = false;

            // don't send flag (that's up to parent)
            // just close connection right away and move on
            closeConnection();
            }
        }
    }






// singleton
static GameEndState state;


GameState *gameEndState = &state;
