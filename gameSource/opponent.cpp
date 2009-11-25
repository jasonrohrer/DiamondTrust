
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



void sendOpponentMessage( unsigned char *inMessage, unsigned int inLength ) {
    if( localAI ) {
        setEnemyMove( inMessage, inLength );
        }
    else {
        sendMessage( inMessage, inLength );
        }
    }



unsigned char *getOpponentMessage( unsigned int *outLength ) {
    if( localAI ) {
        return getAIMove( outLength );
        }
    else {
        return getMessage( outLength );
        }
    }

