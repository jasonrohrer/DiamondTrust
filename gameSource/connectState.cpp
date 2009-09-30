#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "gameStats.h"
#include "units.h"

#include "minorGems/util/stringUtils.h"


static char stateDone = false;

extern Button *parentButton;
extern Button *childButton;
extern char *statusMessage;
extern char *statusSubMessage;

static char isParent;
static char sentMessage, gotMessage;


// wait at least 2 seconds between wait and display
static int stepsSinceConnectTry = 0;
static int minSteps = 30;



class ConnectState : public GameState {
    public:
        ConnectState();
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        

        
        virtual ~ConnectState();
        
    protected:
        char *mMessage;
    };


ConnectState::ConnectState() 
        : mMessage( NULL ) {
    }

ConnectState::~ConnectState() {
    
    if( mMessage != NULL ) {
        delete [] mMessage;
        }
    }

static char connecting = false;



void ConnectState::clickState( int inX, int inY ) {

 
    if( ! connecting ) {
        if( parentButton->getPressed( inX, inY ) ) {
            isParent = true;
            
            acceptConnection();
            char *serverAddress = getLocalAddress();
            
         
            if( mMessage != NULL ) {
                delete [] mMessage;
                }
            
            if( serverAddress != NULL ) {
                mMessage = 
                    autoSprintf( 
                        "%s%s",
                        translate( "phaseSubStatus_waitingConnectOn" ), 
                        serverAddress );

                delete [] serverAddress;
                }
            else {
                mMessage = 
                    stringDuplicate( 
                        translate( "phaseSubStatus_waitingConnect" ) );
                }
            
            statusSubMessage = mMessage;
            connecting = true;
            stepsSinceConnectTry = 0;
            }
        else if( childButton->getPressed( inX, inY ) ) {
            isParent = false;

            connectToServer( NULL );

                   
            statusSubMessage = 
                translate( "phaseSubStatus_connectingToParent" );
            connecting = true;
            stepsSinceConnectTry = 0;
            }
        }
    
    
    }



void ConnectState::stepState() {
    stepsSinceConnectTry ++;
    
    if( connecting && stepsSinceConnectTry > minSteps ) {
        

        if( checkConnectionStatus() == -1 ) {
            statusSubMessage = 
                translate( "phaseSubStatus_connectFailed" );
            return;
            }
        
        if( checkConnectionStatus() == 0 ) {
            // still trying
            return;
            }
        
        if( checkConnectionStatus() == 1 ) {
            
            if( isParent && ! sentMessage ) {
                // send inspector's starting region
                unsigned char message[1];
                message[0] = getUnit( numUnits - 1 )->mRegion;

                sendMessage( message, 1 );
                
                sentMessage = true;
                
                // connected!
                stateDone = true;
                }
            else if( !isParent && ! gotMessage ) {
                
                unsigned int messageLength;
                unsigned char *message = getMessage( &messageLength );
                
                if( message != NULL ) {
                
                    if( messageLength != 1 ) {
                        printOut( "Bad message length from opponent\n" );
                        stateDone = true;

                        delete [] message;
                        return;
                        }

                    // set inspector region
                    getUnit( numUnits - 1 )->mRegion = message[0];
                    getUnit( numUnits - 1 )->mDest = message[0];
                    

                    delete [] message;

                    gotMessage = true;
                
                    // connected!
                    stateDone = true;
                    }                
                
                }
            

            }
        }            
    }



void ConnectState::drawState() {
    
    if( !connecting ) {
        
        parentButton->draw();
        childButton->draw();
        }
    
    }





void ConnectState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_connect" );
    statusSubMessage = translate( "phaseSubStatus_parentOrChild" ); 
    
    isParent = false;
    sentMessage = false;
    gotMessage = false;
    }






// singleton
static ConnectState state;


GameState *connectState = &state;

