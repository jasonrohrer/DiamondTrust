#include "GameState.h"
#include "units.h"
#include "map.h"
#include "Button.h"
#include "common.h"
#include "salePicker.h"
#include "gameStats.h"
#include "Font.h"
#include "colors.h"


#include "minorGems/util/stringUtils.h"



//static int activeUnit = -1;
static char stateDone = false;
static char sentInitialMove = false;
static char gotInitialMove = false;
static char sentMove = false;
static char gotMove = false;

static int stepsSinceSentMove = 0;
static int minSteps = 30;


static char takingPicture = false;
static char pictureFrameReady = false;
static char receivingPicture = true;


extern int pictureDisplaySpriteID;
extern int pictureDisplayW;
extern int pictureDisplayH;
extern int pictureDisplaySpriteW;
extern int pictureDisplaySpriteH;

extern int pictureSendSpriteID;
extern int pictureSendW;
extern int pictureSendH;
extern int pictureSendSpriteW;
extern int pictureSendSpriteH;

extern char pictureSendSpriteSet;


static unsigned char *pictureDisplayData = NULL;

static unsigned char *pictureSendData = NULL;
static unsigned char *pictureReceiveData = NULL;
static int pictureNumBytesReceived = 0;
static int pictureReceiveTotalBytes = 0;

static int pictureCountDown = 0;
static int pictureStepsUntilTick = 30;



extern Button *doneButton;
extern char *statusMessage;
extern char *statusSubMessage;



extern Font *font16;





class SellDiamondsState : public GameState {
    public:

        SellDiamondsState() {
            mStateName = "SellDiamondsState";
            }
        
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




static unsigned char *expandImage( unsigned char *inSource,
                                   int inSourceW, int inSourceH, 
                                   int inDestW, int inDestH ) {
    
    unsigned char *dest = new unsigned char[ inDestW * inDestH ];
    
    memset( dest, 0, (unsigned int)( inDestW * inDestH ) );
    
    for( int y=0; y<inSourceH; y++ ) {
        for( int x=0; x<inSourceW; x++ ) {
            dest[ y * inDestW + x ] = inSource[ y * inSourceW + x ];
            }
        }
    
    return dest;
    }




static void sendMoveMessage() {
    // send our move as a message

    // 3 chars
    // 0 = number sold
    // 1 = image present in subsequent messages?
    // 2 = number of 300-byte messages forthcoming to contain image
    int messageLength = 3;
    unsigned char message[ 3 ];

    message[0] = (unsigned char)getPickerSale();
    int numFutureMessages = 0;
    
    if( pictureSendData != NULL ) {
        message[1] = 1;
        int dataLength = pictureSendW * pictureSendH;
        numFutureMessages = dataLength / 300;
        message[2] = (unsigned char)numFutureMessages;
        }
    else {
        message[1] = 0;
        message[2] = 0;
        }
    
        

    printOut( "Sending message: " );
    for( int i=0; i<messageLength; i++ ) {
        printOut( "%d,", message[i] );
        }
    printOut( "\n" );
            
    sendOpponentMessage( message, (unsigned int)messageLength );


    // now pack image into messages and send them
    if( pictureSendData != NULL ) {
        for( int i=0; i<numFutureMessages; i++ ) {
            int offset = i * 300;
            
            unsigned char *pointer = &( pictureSendData[offset] );
            
            sendOpponentMessage( pointer, 300 );
            }

        delete [] pictureSendData;
        pictureSendData = NULL;
        }        
    }


// returns total sold by opponent, or -1 if message not received
static int getMoveMessage() {
    unsigned int messageLength;
    unsigned char *message = getOpponentMessage( &messageLength );
    //printOut( "trying to receive message\n" );
        
    if( message != NULL ) {
        // got move!            

        // unpack it
        if( messageLength != 3 ) {
            printOut( "Bad message length from opponent, %d\n",
                      messageLength );
            stateDone = true;

            delete [] message;
            return -1;
            }
            
        int returnVal = (int)message[0];
        
        setPlayerNumToSell( 1, returnVal );
        
        if( message[1] ) {
            // image forthcoming
            int numPixels = pictureSendW * pictureSendH;
            
            
            // make sure size matches
            int numMessages = message[2];
            
            if( numMessages * 300 != numPixels ) {
                printOut( "Bad camera image length from opponent\n" );
                stateDone = true;
                
                delete [] message;
                return -1;
                }
            
            pictureReceiveData = 
                new unsigned char[ numPixels ];
            
            memset( pictureReceiveData, 0, (unsigned int)numPixels );


            pictureNumBytesReceived = 0;            
            pictureReceiveTotalBytes = numPixels;
            receivingPicture = true;
            }
        


        delete [] message;
        
        return returnVal;
        }

    return -1;
    }

        


static char pickingSale = false;
static char waitingForDone = false;




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
            waitingForDone = false;
            
            int saleNumber = getPickerSale();
            
            if( saleNumber == 0 ||
                ! isCameraSupported() ) {
                
                statusSubMessage = 
                    translate( "phaseSubStatus_waitingOpponent" );

                sendMoveMessage();
                stepsSinceSentMove = 0;
            
                sentInitialMove = true;
                }
            else {
                // accessing vault and camera active
                // start picture taking process
                takingPicture = true;
                // avoid drawing sprite until we add first new data to it
                pictureFrameReady = false;

                pictureDisplayData = 
                    new unsigned char[ pictureDisplayW * pictureDisplayH ];
                startCamera();

                statusSubMessage = 
                    translate( "phaseSubStatus_sellDiamondsPicture" );
                }            
            }
        }
    else if( ( sentInitialMove && gotInitialMove ) && !sentMove ) {
        if( doneButton->getPressed( inX, inY ) ) {
            pickingSale = false;
            waitingForDone = false;
            
            statusSubMessage = translate( "phaseSubStatus_waitingOpponent" );
            
            sendMoveMessage();
            stepsSinceSentMove = 0;

            sentMove = true;
            }
        }
    
    
    
    }







// called after move received or after move+picture received
static void processInitialMove() {

    if( getPlayerDiamonds( 0 ) > 0 &&
        ( isOpponentHomeBribed() || isPlayerHomeKnownBribed() ) ) {
        // show opponent sale to player and let player adjust

        statusMessage = translate( "phaseStatus_sellPeek" );
        statusSubMessage = 
            translate( "phaseSubStatus_sellDiamondsAdjust" );

        if( isOpponentHomeBribed() ) {
            // only peek at opponent if opponent home is compromised
                    
            peekSale();
                    
            // try this:
            // if peeking, show computed earnings and let player
            // adjust to test strategies before picking one
            finishSale();
            }
        // else allow player to adjust, but don't peek
                
        pickingSale = true;
        waitingForDone = true;
        }
    else {
        // no new info, 
        // OR
        // no room to adjust (we have 0 diamonds),
        // no need to adjust!

        // send final move to opponent right away
        statusSubMessage = 
            translate( "phaseSubStatus_waitingOpponent" );
            
        sendMoveMessage();
                
        sentMove = true;
        }                
    }



static int playerEarnings[2] = {0,0};
static int numSold[2] = {0,0};

static int stepsSinceEarningTick = 0;
static int minEarningsSteps = 10;


void SellDiamondsState::stepState() {

    if( takingPicture ) {
        getFrame( pictureDisplayData );
        
        unsigned char *newSpriteData = expandImage( pictureDisplayData,
                                                    pictureDisplayW,
                                                    pictureDisplayH,
                                                    pictureDisplaySpriteW,
                                                    pictureDisplaySpriteH );
        
        replaceSprite256( pictureDisplaySpriteID, newSpriteData,
                          pictureDisplaySpriteW, pictureDisplaySpriteH );
        
        delete [] newSpriteData;
        
        // first data added
        pictureFrameReady = true;
        

        pictureStepsUntilTick--;
        if( pictureStepsUntilTick == 0 ) {
            // tick
            pictureCountDown--;
            pictureStepsUntilTick = 30;
            
            if( pictureCountDown == 0 ) {
                // take picture

                snapPicture( pictureDisplayData );
                
                unsigned char *newSpriteData = 
                    expandImage( pictureDisplayData,
                                 pictureDisplayW,
                                 pictureDisplayH,
                                 pictureDisplaySpriteW,
                                 pictureDisplaySpriteH );
        
        
                replaceSprite256( pictureDisplaySpriteID, newSpriteData,
                                  pictureDisplaySpriteW, 
                                  pictureDisplaySpriteH );
                
                delete [] newSpriteData;


                // convert to send size
                int scaleFactor = pictureDisplayW / pictureSendW;
                pictureSendData = 
                    new unsigned char[ pictureSendW * pictureSendH ];
                
                for( int y=0; y<pictureDisplayH; y++ ) {
                    int sendY = y / scaleFactor;
                    for( int x=0; x<pictureDisplayW; x++ ) {    
                        int sendX = x / scaleFactor;
                        
                        pictureSendData[ sendY * pictureSendW + sendX ] =
                            pictureDisplayData[ y * pictureDisplayW + x ];
                        }
                    }

                delete [] pictureDisplayData;
                pictureDisplayData = NULL;
                
                stopCamera();
                
                
                takingPicture = false;
                
                // picture ready, send move
                statusSubMessage = 
                    translate( "phaseSubStatus_waitingOpponent" );

                sendMoveMessage();
                stepsSinceSentMove = 0;
            
                sentInitialMove = true;
                }
            }
        }


    stepsSinceSentMove ++;
    
    if( gotInitialMove && pictureReceiveData != NULL ) {
        
        unsigned int messageLength;
        unsigned char *message = getOpponentMessage( &messageLength );
        
        if( message != NULL ) {
            
            if( messageLength != 300 ) {
                printOut( "Bad partial image message length from opponent\n" );
                stateDone = true;

                delete [] message;
                return;
                }
        
            // add to image
            memcpy( &( pictureReceiveData[ pictureNumBytesReceived ] ),
                    message, 300 );
            pictureNumBytesReceived += 300;
            
            delete [] message;


            // DON't show progress, message-by-message, too much for DS
            // platform (results in tears on other textures)


            if( pictureNumBytesReceived == pictureReceiveTotalBytes ) {
                // done, replace sprite
                unsigned char *newSpriteData = 
                    expandImage( pictureReceiveData,
                                 pictureSendW,
                                 pictureSendH,
                                 pictureSendSpriteW,
                                 pictureSendSpriteH );

                // use safe replacement
                replaceSprite256( pictureSendSpriteID, newSpriteData,
                                  pictureSendSpriteW, 
                                  pictureSendSpriteH, true );
            
                pictureSendSpriteSet = true;
            
                delete [] newSpriteData;


                delete [] pictureReceiveData;
                pictureReceiveData = NULL;

                // now done receiving initial move
                processInitialMove();
                }
            else {
                // still waiting for more
                return;
                }
            

            
            }
        else {
            // still waiting for more
            return;
            }
        }
    


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

            if( pictureReceiveData == NULL ) {
                processInitialMove();
                }
            // else wait for picture to arrive
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
                        
                        if( i==1 && ! isOpponentMoneyVisible() ) {
                            // no need to step it, because we can't see 
                            // it anyway
                            // (and we would have to wait w/ no activity
                            //  on the screen if opp earns more money than
                            //  us )

                            addPlayerMoney( 1, playerEarnings[1] );
                            playerEarnings[1] = 0;
                            }
                        

                        changedSome = true;
                        stepsSinceEarningTick = 0;
                        }
                    }
                }
            
        

            if( !changedSome ) {
                // leave display up after state end
                // showSale( false );
                
                //setPlayerNumToSell( 0, 0 );
                //setPlayerNumToSell( 1, 0 );
                
                stateDone = true;
                }
            }
        
        }
    
    
    }



void SellDiamondsState::drawState() {
    // don't draw sprite until new data ready (or else 1 frame
    //   from last picture-taking session is shown)
    
    if( takingPicture  && pictureFrameReady ) {
        // cover map with camera display
        int xOffset = (256 - 160)/2;
        int yOffset = (192 - 120)/2;
        

        drawSprite( pictureDisplaySpriteID, 
                    xOffset, yOffset, 
                    white );
                
        startNewSpriteLayer();
        
        char *countString = autoSprintf( "%d", pictureCountDown );
    
        font16->drawString( countString, 
                            xOffset + pictureDisplayW / 2, 
                            yOffset + pictureDisplayH + 2,
                            white, 
                            alignCenter );

        delete [] countString;


        char *headerString = translate( "camera_securityHeader" );
            
        font16->drawString( headerString,
                            xOffset + pictureDisplayW / 2,
                            yOffset - 32,
                            white,
                            alignCenter );

        // show a known breech at the last minute
        if( isPlayerHomeKnownBribed() &&
            pictureCountDown <= 2 ) {
            char *breechString = translate( "camera_securityCompromised" );
            
            font16->drawString( breechString,
                                xOffset + pictureDisplayW / 2,
                                yOffset - 16,
                                red,
                                alignCenter );
            }
        }
    else {
        
        
        drawMap();
        startNewSpriteLayer();
        
        drawUnits();
        
        if( waitingForDone ) {
            doneButton->draw();
            }
        
        
        if( pickingSale ) {
            drawSalePicker( 24, 161 );
            }    
        }
    
    }





void SellDiamondsState::enterState() {
    stateDone = false;
    sentInitialMove = false;
    gotInitialMove = false;
    sentMove = false;
    gotMove = false;

    takingPicture = false;
    pictureFrameReady = false;
    receivingPicture = false;
    pictureSendSpriteSet = false;
    pictureCountDown = 5;
    
    pickingSale = true;
    waitingForDone = true;
    setPickerSale( 0 );

    // reset sale numbers
    setPlayerNumToSell( 0, 0 );
    setPlayerNumToSell( 1, 0 );


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



    // clear the sprite to start
    int numPixels =  pictureSendSpriteW * pictureSendSpriteH;
    
    unsigned char *newSpriteData = new unsigned char[ numPixels ];
    memset( newSpriteData, 0, (unsigned int)numPixels );
                            
    // use safe replacement
    replaceSprite256( pictureSendSpriteID, newSpriteData,
                      pictureSendSpriteW, 
                      pictureSendSpriteH, true );
            
    delete [] newSpriteData;
    }






// singleton
static SellDiamondsState state;


GameState *sellDiamondsState = &state;

