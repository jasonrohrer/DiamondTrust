#include "GameState.h"
#include "Button.h"
#include "units.h"
#include "map.h"
#include "common.h"
#include "gameStats.h"

//static int activeUnit = -1;
static char stateDone = false;

extern Button *playAgainButton;

extern char *statusMessage;
extern char *statusSubMessage;



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
        
        // back to go back to main menu
        virtual char canStateBeBackedOut() {
            return true;
            }
        

        // destructor?
        //virtual ~GameState();
        
    };








void GameEndState::clickState( int inX, int inY ) {
    
    if( playAgainButton->getPressed( inX, inY ) ) {

        resetStats();
        resetMapDiamondCounts();
        resetUnits();
        // all units are home and empty

        // leave inspector alone
        
        stateDone = true;
        }

    }




void GameEndState::stepState() {
    // no stepping!
    }



void GameEndState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    playAgainButton->draw();
    }





void GameEndState::enterState() {
    stateDone = false;

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






// singleton
static GameEndState state;


GameState *gameEndState = &state;
