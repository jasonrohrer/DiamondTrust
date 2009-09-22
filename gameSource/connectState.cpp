#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "gameStats.h"

#include "minorGems/util/stringUtils.h"


static char stateDone = false;

extern Button *parentButton;
extern Button *childButton;
extern char *statusMessage;
extern char *statusSubMessage;



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
            }
        else if( childButton->getPressed( inX, inY ) ) {

            connectToServer( NULL );

                   
            statusSubMessage = 
                translate( "phaseSubStatus_connectingToParent" );
            connecting = true;
            }
        }
    
    
    }



void ConnectState::stepState() {

    if( connecting ) {
        

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
            
            // connected!
            stateDone = true;
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
   
    }






// singleton
static ConnectState state;


GameState *connectState = &state;

