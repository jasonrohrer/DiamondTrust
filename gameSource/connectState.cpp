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
extern Button *parentServeCloneDownloaButton;
extern Button *childButton;
extern char *statusMessage;
extern char *statusSubMessage;

extern char isParent;
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
        
        virtual char canStateBeBackedOut() {
            return ! isAutoconnecting();
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
        else if( parentServeCloneDownloaButton != NULL &&
                 parentServeCloneDownloaButton->getPressed( inX, inY ) ) {
            
            isParent = true;
            
            acceptCloneDownloadRequest();
            
         
            if( mMessage != NULL ) {
                delete [] mMessage;
                }
            
            mMessage = 
                stringDuplicate( 
                    translate( "phaseSubStatus_waitingConnect" ) );
            
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

        char showIcon = isCloneBootPossible();
        
        parentButton->draw();

        if( showIcon ) {
            drawWirelessIconNextToButton( parentButton );
            }
        

        if( parentServeCloneDownloaButton != NULL ) {
            parentServeCloneDownloaButton->draw();

            if( showIcon ) {
                drawWirelessIconNextToButton( parentServeCloneDownloaButton );
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
    
    isParent = false;
    sentMessage = false;
    gotMessage = false;

    connecting = false;

    if( isAutoconnecting() ) {
        // clone boot child?
        connecting = true;
        }
    }



void ConnectState::backOutState() {
    if( connecting ) {
        closeConnection();
        }
    };




// singleton
static ConnectState state;


GameState *connectState = &state;

