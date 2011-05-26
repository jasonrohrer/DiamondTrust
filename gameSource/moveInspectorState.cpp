#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "bidPicker.h"
#include "gameStats.h"


//static int activeUnit = -1;
static char stateDone = false;
static char connectionBroken = true;
static char moving = false;
static char waiting = false;
static char sentMove = false;
static char gotMove = false;

static int stepsWaiting = 0;
static int minSteps = 30;

extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;



class MoveInspectorState : public GameState {
    public:

        MoveInspectorState() {
            mStateName = "MoveInspectorState";
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


        // destructor?
        //virtual ~GameState();
        
    };




static void sendMoveMessage() {
    // send our move as a message

    // 1 char
    // inspector dest
    int messageLength = 1;
    unsigned char message[ 1 ];

    message[0] = (unsigned char)getUnitDestination( numUnits - 1 );
                
    printOut( "Sending message: " );
    for( int i=0; i<messageLength; i++ ) {
        printOut( "%d,", message[i] );
        }
    printOut( "\n" );
            
    sendOpponentMessage( message, (unsigned int)messageLength );
    }




void MoveInspectorState::clickState( int inX, int inY ) {


    // region picked?
    int chosenRegion = getChosenRegion( inX, inY );
            
    if( chosenRegion != -1 ) {
        setUnitDestination( numUnits-1, chosenRegion );
        }

    
    if( moving && !sentMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            
            setAllRegionsNotSelectable();
            
            
            sendMoveMessage();
                        
            statusSubMessage = translate( "phaseSubStatus_moveExecute" );
            
            
            sentMove = true;

            // turn off halo now that move specification done
            setActiveUnit( -1 );

            showAllUnitMoves( true );

            
            // turn off inspector bribe tags for move
            for( int i=0; i<numPlayerUnits*2; i++ ) {
                showInspectorBribe( i, false );
                }
            
            executeUnitMoves();
            }
        }
    
    
    
    }



void MoveInspectorState::stepState() {
    
    stepUnits();
    stepsWaiting++;
    
    if( waiting && !gotMove && stepsWaiting > minSteps ) {
        
        if( checkConnectionStatus() != 1 ) {
            connectionBroken = true;
            stateDone = true;
            return;
            }

        

        unsigned int messageLength;
        unsigned char *message = getOpponentMessage( &messageLength );
        //printOut( "trying to receive message\n" );
        
        if( message != NULL ) {
            // got move!            
            gotMove = true;
            
            // unpack it
            if( messageLength != 1 ) {
                printOut( "Bad message length from opponent\n" );
                stateDone = true;
                
                delete [] message;
                return;
                }
            
            
            setUnitDestination( numUnits-1, (int)message[0] );
            
            delete [] message;
                     

            statusSubMessage = 
                translate( "phaseSubStatus_moveExecute" );
            
            showAllUnitMoves( true );

            // now that destination is known, show result
            showInspectorPanel( true );

            // turn off inspector bribe tags for move
            for( int i=0; i<numPlayerUnits*2; i++ ) {
                showInspectorBribe( i, false );
                }

            executeUnitMoves();
            }        
        }


    if( sentMove || gotMove ) {
        if( unitAnimationsDone() ) {

            // This state ends with inspector panel still visible
            stateDone = true;
            }
        }
    
    
    }



void MoveInspectorState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();
    
    if( moving && !sentMove ) {
        doneButton->draw();
        }    
    }





void MoveInspectorState::enterState() {
    stateDone = false;
    connectionBroken = false;
    
    statusMessage = translate( "phaseStatus_moveInspector" );
            
    moving = false;
    waiting = false;
    
    if( getPlayerBribedInspector() == 0 ) {
        
        
        moving = true;
        setActiveUnit( numUnits - 1 );

        statusSubMessage = translate( "phaseSubStatus_pickInspectorRegion" ); 

        for( int r=2; r<numMapRegions; r++ ) {
            setRegionSelectable( r, true );
            }

        showInspectorPanel( true );
        }
    else {
        waiting = true;
        statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );
        stepsWaiting = 0;
        showInspectorPanel( false );
        }
    
    sentMove = false;
    gotMove = false;
    
    
    showUnitMoves( true );
    }






// singleton
static MoveInspectorState state;


GameState *moveInspectorState = &state;
