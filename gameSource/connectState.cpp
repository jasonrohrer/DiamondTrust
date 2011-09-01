#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "gameStats.h"
#include "units.h"
#include "colors.h"

#include "minorGems/util/stringUtils.h"


static char stateDone = false;

extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;

extern int titleSpriteID;
extern unsigned char titleFade;

extern int wirelessOnSpriteID;



extern Button *parentButton;
extern Button *parentServeCloneDownloadButton;
extern Button *childButton;
extern const char *statusMessage;
extern const char *statusSubMessage;

extern char isHost;
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


        virtual char needsNextButton() {
            return false;
            }        

        
        virtual char canStateBeBackedOut() {
            //return ! isAutoconnecting();
            // allow back button even when autoconnecting as a cloneboot child,
            // so that child doesn't get stuck if parent drops out after
            // serving the ROM.
            return true;
            }

        virtual void backOutState();
        
        virtual ~ConnectState();
        
    protected:
        char *mMessage;
    };


ConnectState::ConnectState() 
        : mMessage( NULL ) {

    mStateName = "ConnectState";
    }

ConnectState::~ConnectState() {
    
    if( mMessage != NULL ) {
        delete [] mMessage;
        }
    }

static char connecting = false;
static char servingCloneBoot = false;

static int lastCloneHostState = 0;

static char connectionFailedReported = false;



static void childStartConnect() {

    isHost = false;
    
    connectToServer( NULL );
    
    
    statusSubMessage = 
        translate( "phaseSubStatus_connectingToParent" );
    connecting = true;
    stepsSinceConnectTry = 0;
    }
    



void ConnectState::clickState( int inX, int inY ) {

 
    if( ! connecting ) {
        if( parentButton->getPressed( inX, inY ) ) {
            isHost = true;
            
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
            
            nextSubState();
            }
        else if( parentServeCloneDownloadButton != NULL &&
                 parentServeCloneDownloadButton->getPressed( inX, inY ) ) {
            
            isHost = true;
            
            acceptCloneDownloadRequest();
            
         
            if( mMessage != NULL ) {
                delete [] mMessage;
                }
            
            mMessage = 
                stringDuplicate( 
                    translate( "phaseSubStatus_waitingConnect" ) );
            
            statusSubMessage = mMessage;
            connecting = true;
            servingCloneBoot = true;
            
            stepsSinceConnectTry = 0;
            
            nextSubState();
            }
        else if( childButton->getPressed( inX, inY ) ) {
            childStartConnect();
            nextSubState();
            }
        }
    
    
    }



void ConnectState::stepState() {
    
    



    if( connecting ) {
        if( titleFade > 0 ) {
        
            if( titleFade >= 8 ) {
                titleFade -= 8;
                }
            else {
                titleFade = 0;
                }
            }
        }
    

    stepsSinceConnectTry ++;
    
    if( connecting && stepsSinceConnectTry > minSteps ) {
        

        if( checkConnectionStatus() == -1 ) {
            if( !connectionFailedReported ) {
                
                printOut( "Failed\n" );
                statusSubMessage = 
                    translate( "phaseSubStatus_connectFailed" );
                nextSubState();
                connectionFailedReported = true;
                }
            
            return;
            }

        if( isHost && servingCloneBoot && 
            ( checkConnectionStatus() == -2 || 
              checkConnectionStatus() == 0 ) ) {
            // hasn't gotten a real connection from child yet
            
            // check and display status

            int cloneState = getCloneHostState();
                
            if( lastCloneHostState != cloneState ) {
                // message change

                // keep advancing sub state throughout various transitions
                // thus, music changes as we receive multiple connection
                // tries, etc.
                // sub state might eventually wrap around back to 'a'.
                nextSubState();

                switch( cloneState ) {
                    case -1:
                        statusSubMessage = 
                            translate( "phaseSubStatus_connectFailed" );
                        break;
                    case 0:
                    case 1:

                        if( lastCloneHostState == 2 ) {
                            // download in progress, but failed?
                            statusSubMessage = translate(
                               "phaseSubStatus_waitAgainAfterFailedTransfer" );
                            }
                        else {
                            // still waiting for first connection
                            statusSubMessage = 
                                translate( "phaseSubStatus_waitingConnect" );
                            }
                        break;
                    case 2:
                        if( mMessage != NULL ) {
                            delete [] mMessage;
                            }
                        mMessage = 
                            autoSprintf(
                               "%s",
                               translate( "phaseSubStatus_sendingDownload" ) );
                        
                        statusSubMessage = mMessage;
                        break;
                    case 3:
                        statusSubMessage = 
                            translate( "phaseSubStatus_waitingReconnect" );
                        break;
                    }

                lastCloneHostState = cloneState;
                }
            }
        


        if( checkConnectionStatus() == 0 ) {
            // still trying
            return;
            }
        
        if( checkConnectionStatus() == 1 ) {
            
            if( isHost && ! sentMessage ) {
                // send inspector's starting region
                unsigned char message[1];
                message[0] = 
                    (unsigned char)( getUnit( numUnits - 1 )->mRegion );

                sendMessage( message, 1 );
                
                sentMessage = true;
                
                // connected!
                stateDone = true;

                // force title to fade all the way out
                // in case it hasn't already
                titleFade = 0;
                }
            else if( !isHost && ! gotMessage ) {
                
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

                    // force title to fade all the way out
                    // in case it hasn't already
                    titleFade = 0;
                    }                
                
                }
            

            }
        } 


           
    }


static void drawWirelessIconNextToButton( Button *inButton ) {
    drawSprite( wirelessOnSpriteID,
                inButton->getCenterX() + 
                inButton->getWidth() / 2 + 4,
                inButton->getCenterY() - 8, 
                white );
    }



void ConnectState::drawState() {
    
    drawSprite( satelliteTopSpriteID, 
                0,0, white );
    drawSprite( satelliteBottomSpriteID, 
                0,satelliteBottomHalfOffset, white );
    
    startNewSpriteLayer();
    
    if( titleFade > 0 ) {
        rgbaColor titleColor = white;
        titleColor.a = titleFade;
        
        drawSprite( titleSpriteID, 
                    0,6, titleColor );
        /*
        drawSprite( titleSpriteID[1], 
                    0,64, titleColor );

        drawSprite( titleSpriteID[2], 
                    0,80, titleColor );
        */
        }
    
    if( !connecting ) {

        char showIcon = shouldShowDSWiFiIcons();
        
        parentButton->draw();

        if( showIcon ) {
            drawWirelessIconNextToButton( parentButton );
            }
        

        if( parentServeCloneDownloadButton != NULL ) {
            parentServeCloneDownloadButton->draw();

            if( showIcon ) {
                drawWirelessIconNextToButton( parentServeCloneDownloadButton );
                }
            }
        
        childButton->draw();

        if( showIcon ) {
            drawWirelessIconNextToButton( childButton );
            }
        }
    
    }





void ConnectState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_connect" );
    if( !isAutoconnecting() ) {
        statusSubMessage = translate( "phaseSubStatus_parentOrChild" ); 
        }
    
    isHost = false;
    sentMessage = false;
    gotMessage = false;

    connecting = false;
    servingCloneBoot = false;
    lastCloneHostState = 0;
    
    connectionFailedReported = false;


    if( isAutoconnecting() ) {
        // clone boot child?
        childStartConnect();
        }
    }



void ConnectState::backOutState() {
    if( connecting ) {

        if( servingCloneBoot ) {
            cancelCloneHosting();
            }
        else {
            // just close the connection directly
            closeConnection();
            }
        }
    };




// singleton
static ConnectState state;


GameState *connectState = &state;

