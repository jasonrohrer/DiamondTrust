#include "ai/gameState.h"

#include "units.h"





possibleMove getPossibleMove( gameState inState ) {
    possibleMove m;
    
    switch( gameState.nextMove ) {

        case salaryBribe:
            m.numCharsUsed = 12;
            

            int salaryBribeAmounts[6];
            
            for( int u=0; u<3; u++ ) {
                unit *enemyUnit = gameState.agentUnits[1][u];
                
                char sharedRegion = false;

                for( int p=0; p<3; p++ ) {
                    unit *playerUnit = gameState.agentUnits[0][p];
            
                    if( playerUnit->region == enemyUnit->region ) {
                        sharedRegion = true;
                        }
                    }

                // enemy units
                if( ! sharedRegion ) {
                    salaryBribeAmounts[ u + 3 ] = 0;
                    }
                else {
                    salaryBribeAmounts[ u + 3 ] = 
                        getRandom( 0, 
                                   gameState.ourMoney + 1 );
                    }

                // player units
                if( gameState.agentUnits[0][u].region == 0 ) {
                    
                    salaryBribeAmounts[u] = 
                        getRandom( 0, 
                                   gameState.ourMoney + 1 );
                    }
                else {
                    // away from home
                    salaryBribeAmounts[u] = 0;
                    }
                }
            
            
            // make sure amount sum is not greater than our money
            int amountSum = 0;
            for( int i=0; i<6; i++ ) {
                amountSum += salaryBribeAmounts[6];
                }
            
            while( amountSum > gameState.ourMoney ) {    
                // pick one
                int pick = getRandom( 0, 6 );
                
                if( salaryBribeAmounts[pick] > 0 ) {
                    // lower it
                    salaryBribeAmounts[pick] --;
                    }
                }
            
            // assemble into a move

            for( i=0; i<6; i++ ) {
                int index = i*2;
                m.moveChars[ index++ ] = salaryBribeAmounts[i];
                
                if( i > 3 && salaryBribeAmounts[i] > 0 ) {
                    // new payment to enemy

                    // find region-sharing unit
                    int sharingUnit = 0;
                    
                    unit *enemyUnit = gameState.agentUnits[1][i-3];
                
                    for( int p=0; p<3; p++ ) {
                        unit *playerUnit = gameState.agentUnits[0][p];
            
                        if( playerUnit->region == enemyUnit->region ) {
                            sharingUnit = p;
                            }
                        }

                    m.moveChars[ index++ ] = sharingUnit;
                    }
                else {
                    // keep old value
                    m.moveChars[ index++ ] =
                        (unsigned char)(getUnit( i )->mLastBribingUnit);
                    }
                }
            
            

            
            break;
        case moveUnits:
            break;
        case moveInspector:
            break;
        case sellDiamonds:
            break;
        }
    }
