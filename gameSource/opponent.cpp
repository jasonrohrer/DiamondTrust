
#include "opponent.h"
#include "platform.h"
#include "ai/ai.h"


static char localAI = false;




void initOpponent( char inLocalAI ) {
    localAI = inLocalAI;
    
    if( localAI ) {
        initAI();
        }
    }


void resetOpponent() {
    localAI = false;
    }



void freeOpponent() {
    if( localAI ) {
        freeAI();
        localAI = false;
        }
    }


void stepOpponent() {
    if( localAI ) {
        stepAI();
        }
    // else no need to step network
    }

    


int checkOpponentConnectionStatus() {
    if( localAI ) {
        return 1;
        }
    else {
        return checkConnectionStatus();
        }
    }



void closeOpponentConnection() {
    if( ! localAI ) {
        closeConnection();
        }
    }



void sendOpponentMessage( unsigned char *inMessage, unsigned int inLength ) {
    if( localAI ) {
        setEnemyMove( inMessage, inLength );
        }
    else {
        sendMessage( inMessage, inLength );
        }
    }


#include <stdio.h>


unsigned char *getOpponentMessage( unsigned int *outLength ) {
    if( localAI ) {
        
        // since we're polling AI, we're waiting on it
        // tell it to use more CPU to speed up
        toggleAICPUMode( true );
        
        unsigned char *returnValue = getAIMove( outLength );

        if( returnValue != NULL ) {
            // we got a result from AI, so we're no longer waiting for it
            
            // tell it to use less CPU again so that UI is responsive
            toggleAICPUMode( false );
            }

        return returnValue;
        }
    else {
        return getMessage( outLength );
        }
    }

