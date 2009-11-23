#include "ai/gameState.h"

#include "units.h"
#include "map.h"





possibleMove getPossibleMove( gameState inState ) {
    possibleMove m;
    
    switch( inState.nextMove ) {



        case salaryBribe: {
            
            // 2 chars per unit (salary or bribe, plus bribing unit)
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
                        // unknown

                        // use value from current game
                        knownValue = getUnit( i )->mLastBribingUnit;
                        }
                    else if( knownValue == 3 ) {
                        // known to be no bribing unit
                        knownValue = -1;
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




        case moveUnits: {
            // 3 chars per player unit
            // dest, bid, bribe
            m.numCharsUsed = 9;

            int dest[3];
            int bid[3];
            int bribe[3];

            int totalSpent = 0;

            int u;
            for( u=0; u<3; u++ ) {
                dest[u] = getRandom( 7 );
                if( dest[u] > 0 ) {
                    // add 1 to make total range [0, 2..7]
                    dest[u]++;
                    }

                bid[u] = 0;
                if( dest[u] > 1 ) {
                    // can bid here
                    bid[u] = getRandom( inState.ourMoney + 1 );
                    totalSpent += bid[u];
                    }

                bribe[u] = 0;
                if( dest[u] == inState.inspectorRegion ) {
                    bribe[u] = getRandom( inState.ourMoney + 1 );
                    totalSpent += bribe[u];
                    }

                // cost of trip
                if( dest[u] != inState.agentUnits[0][u].region ) {
                    totalSpent++;

                    if( dest[u] == 0 ) {
                        // flying home costs double
                        totalSpent++;
                        }
                    }
                }
            

            while( totalSpent > inState.ourMoney ) {
                // spending too much
                
                // pick one to reduce
                u = getRandom( 3 );
                
                // priority:  trip itself, bribe, bid
                if( bid[u] > 0 ) {
                    bid[u] --;
                    totalSpent --;
                    }
                else if( bribe[u] > 0 ) {
                    bribe[u] --;
                    totalSpent --;
                    }
                else {
                    // cancel trip?
                    if( dest[u] != inState.agentUnits[0][u].region ) {
                        totalSpent --;
                        
                        if( dest[u] == 0 ) {
                            // flying home costs double
                            totalSpent --;
                            }

                        // stay where it is
                        dest[u] = inState.agentUnits[0][u].region;
                        }
                    }                    
                }
            
            
            // within budget limits now
            
            // pack into char string
            int index = 0;
            for( u=0; u<3; u++ ) {
                m.moveChars[ index++ ] = (unsigned char)( dest[u] );
                m.moveChars[ index++ ] = (unsigned char)( bid[u] );
                m.moveChars[ index++ ] = (unsigned char)( bribe[u] );
                }
            }
            break;



        case moveInspector:
            // 1 char, inspector dest
            m.numCharsUsed = 1;
            
            // avoid player regions
            m.moveChars[ 0 ] = getRandom( 6 ) + 2;
            break;



        case sellDiamonds:
            // 3 chars
            // 0 = number sold
            // 1 = image present in subsequent messages?
            // 2 = number of 300-byte messages forthcoming to contain image

            // always skip sending image
            m.numCharsUsed = 3;
            m.moveChars[0] = 
                (unsigned char)getRandom( inState.ourDiamonds + 1 );
            m.moveChars[1] = 0;
            m.moveChars[2] = 0;
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
    
    return makeRange( point );
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
    

    result.enemyMoney = makeRange( netMoney );
    

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





gameState getMirrorState( gameState inState ) {
    gameState result = inState;
    
    result.ourMoney = inState.enemyMoney;
    result.ourDiamonds = inState.enemyDiamonds;
    result.enemyMoney = inState.ourMoney;
    result.enemyDiamonds = inState.ourDiamonds;

    result.knownEnemyTotalMoneyReceived = inState.knownOurTotalMoneyReceived;
    result.knownEnemyTotalMoneySpent = inState.knownOurTotalMoneySpent;

    for( int u=0; u<3; u++ ) {
        result.agentUnits[0][u] = inState.agentUnits[1][u];
        result.agentUnits[1][u] = inState.agentUnits[0][u];
        }

    return result;
    }



int playRandomGameUntilEnd( gameState inState ) {
    
    // skip sell diamonds during last month and fly everyone home
    while( inState.monthsLeft > 0 || inState.nextMove != sellDiamonds ) {
        
        
        possibleMove ourMove = getPossibleMove( inState );
        
        gameState mirror = getMirrorState( inState );
        possibleMove enemyMove = getPossibleMove( mirror );
        
        inState = 
            stateTransition( inState,
                             ourMove.moveChars,
                             ourMove.numCharsUsed,
                             enemyMove.moveChars,
                             enemyMove.numCharsUsed,
                             false );        
        }
    
    // on sell diamonds state in month zero
    // don't sell
    // instead, "fly" everyone home to get diamond totals
    int ourTotal = inState.ourDiamonds;
    int enemyTotal = inState.enemyDiamonds;
    
    for( int u=0; u<3; u++ ) {
        ourTotal += inState.agentUnits[0][u].diamonds;
        enemyTotal += inState.agentUnits[1][u].diamonds;
        }
    
    int diff = ourTotal - enemyTotal;
    
    if( diff == 0 ) {
        // diff money instead
        diff = inState.ourMoney.t - inState.enemyMoney.t;
        }

    return diff;
    }





static intRange processBribe( intRange inOldBribeTotal,
                              int inOurRegion,
                              unit inOpponentUnits[3],
                              int inNewBribe,
                              intRange inOpponentMoney,
                              char inPreserveHiddenInfo ) {

    intRange totalBribe = inOldBribeTotal;
    
    totalBribe.t += inNewBribe;
    
    if( ! inPreserveHiddenInfo ) {
        totalBribe.hi += inNewBribe;
        totalBribe.lo = totalBribe.hi;
        }
    else {
        // how much did they bribe us?
        
        // upper limit goes up if we're in a region with
        // enemy
        char sharedRegion = false;
        for( int e=0; e<3; e++ ) {
            if( inOpponentUnits[e].region == inOurRegion ) {
                sharedRegion = true;
                }
            }
        
        if( sharedRegion ) {
            // enemy might have spent all its money bribing us
            totalBribe.hi += inOpponentMoney.hi;
            }
        }

    return totalBribe;
    }



static intRange processSalary( intRange inOldSalaryTotal,
                               int inUnitRegion,
                               int inPaymentRegion,
                               int inNewSalary,
                               intRange inOpponentMoney,
                               char inPreserveHiddenInfo ) {

    intRange totalSalary = inOldSalaryTotal;
    
    totalSalary.t += inNewSalary;
    
    if( ! inPreserveHiddenInfo ) {
        totalSalary.hi += inNewSalary;
        totalSalary.lo = totalSalary.hi;
        }
    else {
        // how much did they pay unit?
        
        // upper limit goes up if unit is in payment region
        if( inUnitRegion == inPaymentRegion ) {
            // enemy might have spent all its money bribing us
            totalSalary.hi += inOpponentMoney.hi;
            }
        }

    return totalSalary;
    }





gameState stateTransition( gameState inState, 
                           unsigned char *ourMove,
                           int ourLength,
                           unsigned char *enemyMove,
                           int enemyLength,
                           char inPreserveHiddenInfo ) {
    gameState result = inState;
    
    switch( inState.nextMove ) {




        case salaryBribe: {
            if( ourLength != 12 && enemyLength != 12 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return;
                }
            
            for( int u=0; u<3; u++ ) {
                
                // FIXME:  need to take intRange.t into account here
                // to track true values
                // this will also help to make code symmetrical for both
                // sides (use hi and lo to track knowledge that opponent
                // has about these units, in either case

                // we KNOW how much we've paid our own units
                result.agentUnits[0][u].totalSalary.hi += 
                    ourMove[u * 2];
                result.agentUnits[0][u].totalSalary.lo =
                    result.agentUnits[0][u].totalSalary.hi;
                
                // spend our money
                result.ourMoney -= ourMove[u * 2];
                
                    
                // we might not be sure about how much we've been
                // bribed by enemy units
                result.agentUnits[0][u].totalBribe =
                    processBribe( result.agentUnits[0][u].totalBribe,
                                  result.agentUnits[0][u].region,
                                  result.agentUnits[1],
                                  enemyMove[ (u+3) * 2 ],
                                  result.enemyMoney,
                                  inPreserveHiddenInfo );
                    
                if( !inPreserveHiddenInfo ) {
                    // spend enemy money
                    result.enemyMoney.hi -= enemyMove[ (u+3) * 2 ];
                    result.enemyMoney.lo = result.enemyMoney.hi;

                    // remember who bribed this unit
                    if( result.agentUnits[0][u].totalBribe.lo > 0 ) {
                            
                        result.agentUnits[0][u].opponentBribingUnit =
                            enemyMove[ (u+3) * 2  + 1 ];
                        }
                    }
                    
                    
                // we might not be sure how much enemy unit is
                // paying in salary
                if( !inPreserveHiddenInfo ) {
                    result.agentUnits[1][u].totalSalary.hi += 
                        enemyMove[u * 2];
                    result.agentUnits[1][u].totalSalary.lo =
                        result.agentUnits[1][u].totalSalary.hi;

                    // spend enemy money
                    result.enemyMoney.hi -= enemyMove[u * 2];
                    result.enemyMoney.lo = result.enemyMoney.hi;
                    }
                else {
                    // how much did they pay unit?
        
                    // upper limit goes up if unit is in payment region

                    if( result.agentUnits[1][u].region == 1 ) {
                        // at home

                        // enemy might have spent all its money on salary
                        result.agentUnits[1][u].totalSalary.hi += 
                            inOpponentMoney.hi;
                        }
                    }

                    

                // we KNOW how much we've bribed their units
                result.agentUnits[1][u].totalBribe.hi += 
                    ourMove[(u+3) * 2];
                result.agentUnits[1][u].totalBribe.lo =
                    result.agentUnits[1][u].totalBribe.hi;
                
                // spend our money
                result.ourMoney -= ourMove[(u+3) * 2];
                
                if( result.agentUnits[1][u].totalBribe.hi > 0 ) {
                    result.agentUnits[1][u].opponentBribingUnit = 
                        ourMove[(u+3) * 2 + 1];
                    }
                }
            

            result.nextMove = moveUnits;
            }
            break;





        case moveUnits: {

            // FIXME:  process unit moves here (switch their regions_


            // FIXME:  spend unit bids/bribes here


            // FIXME:  save unit bids/bribes to be used in moveInspector
            //         state (to determine who moves inspector and who
            //         collects diamonds

            result.nextMove = moveInspector;
            }
            break;




        case moveInspector: {
            if( ourLength != 1 && enemyLength != 1 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return;
                }
            
            // FIXME:  only pay attention to move from player that
            // actually won
            

            // FIXME:  process diamond bids/collection here

            result.nextMove = sellDiamonds;
            }
            break;




        case sellDiamonds: {
            if( ourLength != 3 && enemyLength != 3 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return;
                }
            
            result.ourDiamonds -= ourMove[0];
            result.enemyDiamonds -= enemyMove[0];
            
            int totalSold = ourMove[0] + enemyMove[0];

            int ourIncrease = 18;
            if( ourMove[0] > ) {
                ourIncrease= ( ourMove[0] * 24 ) / totalSold;
                }
            ourMoney += ourIncrease;
            knownOurTotalMoneyReceived += ourIncrease;
            

            int enemyIncrease = 18;
            if( enemyMove[0] > ) {
                enemyIncrease= ( enemyMove[0] * 24 ) / totalSold;
                }
            enemyMoney.hi += enemyIncrease;
            enemyMoney.lo += enemyIncrease;
            knownEnemyTotalMoneyReceived += enemyIncrease;
            
            // next month
            result.monthsLeft --;
            
            result = accumulateDiamonds( result );
            
            result.nextMove = salaryBribe;
            }
            break;
        }
    

    return result;            
    }



