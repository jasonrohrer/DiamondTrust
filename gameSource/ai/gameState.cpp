#include "ai/gameState.h"

#include "units.h"
#include "map.h"





possibleMove getPossibleMove( gameState inState ) {
    possibleMove m;
    
    switch( inState.nextMove ) {

        case salaryBribe: {
            
            m.numCharsUsed = 12;
            

            int salaryBribeAmounts[6];
            
            for( int u=0; u<3; u++ ) {
                unit enemyUnit = inState.agentUnits[1][u];
                
                char sharedRegion = false;

                for( int p=0; p<3; p++ ) {
                    unit playerUnit = inState.agentUnits[0][p];
            
                    if( playerUnit.region == enemyUnit.region ) {
                        sharedRegion = true;
                        }
                    }

                // enemy units
                if( ! sharedRegion ) {
                    salaryBribeAmounts[ u + 3 ] = 0;
                    }
                else {
                    salaryBribeAmounts[ u + 3 ] = 
                        getRandom( inState.ourMoney + 1 );
                    }

                // player units
                if( inState.agentUnits[0][u].region == 0 ) {
                    
                    salaryBribeAmounts[u] = 
                        getRandom( inState.ourMoney + 1 );
                    }
                else {
                    // away from home
                    salaryBribeAmounts[u] = 0;
                    }
                }
            
            
            // make sure amount sum is not greater than our money
            int amountSum = 0;
            int i;
            for( i=0; i<6; i++ ) {
                amountSum += salaryBribeAmounts[6];
                }
            
            while( amountSum > inState.ourMoney ) {    
                // pick one
                int pick = getRandom( 6 );
                
                if( salaryBribeAmounts[pick] > 0 ) {
                    // lower it
                    salaryBribeAmounts[pick] --;
                    amountSum--;
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
                    
                    unit enemyUnit = inState.agentUnits[1][i-3];
                
                    for( int p=0; p<3; p++ ) {
                        unit playerUnit = inState.agentUnits[0][p];
            
                        if( playerUnit.region == enemyUnit.region ) {
                            sharingUnit = p;
                            }
                        }

                    m.moveChars[ index++ ] = sharingUnit;
                    }
                else {
                    // keep old value
                    int knownValue =
                        inState.agentUnits[ i/3 ][ i%3 ].opponentBribingUnit;
                    
                    if( knownValue == -1 ) {
                        // use value from current game
                        knownValue = getUnit( i )->mLastBribingUnit;
                        }
                    else {
                        
                        // else stick with known value, but turn it into [0..5]
                        // range
                        if( i < 3 ) {
                            // value describes an enemy unit
                            knownValue += 3;
                            }
                        }

                    m.moveChars[ index++ ] = (unsigned char)( knownValue );
                    }
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


    return m;
    }




void accumulateDiamonds( gameState *inState ) {
    int monthsLeft = inState->monthsLeft;

    for( int r=0; r<8; r++ ) {
        inState->regionDiamondCounts[r] += 
            getRegionDiamondRate( r, monthsLeft );
        }
    }


// pick a random value in range
static intRange collapseRange( intRange inRange ) {
    int point = inRange.lo + getRandom( ( inRange.hi - inRange.lo ) + 1 );
    
    return makeRange( point, point );
    }



gameState collapseState( gameState inState ) {
    gameState result = inState;
    
    // collapse each enemy unit salary and each player unit bribe
    
    // sum them as we do this
    int salaryBribeSum = 0;
    int u;
    
    for( u=0; u<3; u++ ) {
        // enemy salary
        result.agentUnits[1][u].totalSalary = 
            collapseRange( result.agentUnits[1][u].totalSalary );
        
        salaryBribeSum += result.agentUnits[1][u].totalSalary.hi;
        

        // player unit bribes
        result.agentUnits[0][u].totalBribe = 
            collapseRange( result.agentUnits[0][u].totalBribe );

        salaryBribeSum += result.agentUnits[0][u].totalBribe.hi;
        }
    
    int availableForSalaryBribe = 
        result.knownEnemyTotalMoneyReceived -
        result.knownEnemyTotalMoneySpent;
    
    // make sure it's not too high
    while( salaryBribeSum > availableForSalaryBribe ) {

        // pick one
        u = getRandom( 3 );
        int p = getRandom( 2 );
        
        // look at uncollapsed ranges for room to reduce (can't reduce below
        //  lo  value in original, uncollapsed range)

        if( p == 0 ) {
            
            if( result.agentUnits[p][u].totalBribe.lo > 
                inState.agentUnits[p][u].totalBribe.lo ) {
            
                result.agentUnits[p][u].totalBribe.hi --;
                result.agentUnits[p][u].totalBribe.lo --;
                salaryBribeSum --;
                }
            }
        else {
            if( result.agentUnits[p][u].totalSalary.lo > 
                inState.agentUnits[p][u].totalSalary.lo ) {
            
                result.agentUnits[p][u].totalSalary.hi --;
                result.agentUnits[p][u].totalSalary.lo --;
                salaryBribeSum --;
                }
            }
        }
    
    // we've reduced salaries and bribes into range

    
    // now we know how much money enemy has left
    int netMoney = 
        result.knownEnemyTotalMoneyReceived -
        result.knownEnemyTotalMoneySpent -
        salaryBribeSum;
    

    result.enemyMoney = makeRange( netMoney, netMoney );
    

    // collapse our uncertainty about enemy units bribing player
    // units (do this AFTER we collapse bribe info, to avoid picking
    // false knowledge about a zero bribe)

    for( u=0; u<3; u++ ) {
        int bribingUnit = result.agentUnits[0][u].opponentBribingUnit;
        
        if( bribingUnit == -1 ) {
            // unknown.  Collapse to known

            if( result.agentUnits[0][u].totalBribe.hi > 0 ) {
                bribingUnit = getRandom( 3 );
                }
            else {
                // know that no one bribed this unit
                bribingUnit = 3;
                }
            }
        result.agentUnits[0][u].opponentBribingUnit = bribingUnit;
        }
    
    
    return result;
    }

