#include "ai/gameState.h"

#include "units.h"
#include "map.h"


#include <assert.h>


static void checkStateValid( gameState inState ) {
    for( int p=0; p<2; p++ ) {
        
        for( int u=0; u<3; u++ ) {
            if( inState.agentUnits[p][u].region != 0 && 
                inState.agentUnits[p][u].region != 1 ) {
                for( int o=0; o<3; o++ ) {
                    if( o != u ) {
                        if( inState.agentUnits[p][o].region == 
                            inState.agentUnits[p][u].region ) {
                            printOut( "Units collide in state!\n" );
                            }
                        }
                    }
                }
            }
        }

    for( int u=0; u<3; u++ ) {
        if( inState.agentUnits[0][u].region == 1
            ||
            inState.agentUnits[1][u].region == 0 ) {
            
            printOut( "Units in opposite home regions in state!\n" );
            }
        }
    
    }




possibleMove getPossibleMove( gameState *inState ) {
    // checkStateValid( inState );
    

    possibleMove m;
    
    switch( inState->nextMove ) {



        case salaryBribe: {
            
            // 2 chars per unit (salary or bribe, plus bribing unit)
            m.numCharsUsed = 12;
            

            // pick a random spending cap
            // (otherwise, all the random choices almost always add
            //  up to equal our total money---i.e., we spend everything
            //  we have!)
            int maxTotalToSpend = getRandom( inState->ourMoney.t + 1 );



            int salaryBribeAmounts[6];
            
            for( int u=0; u<3; u++ ) {
                unit enemyUnit = inState->agentUnits[1][u];
                
                char sharedRegion = false;

                for( int p=0; p<3; p++ ) {
                    unit playerUnit = inState->agentUnits[0][u];
            
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
                        getRandom( maxTotalToSpend + 1 );
                    }

                // player units
                if( inState->agentUnits[0][u].region == 0 ) {
                    
                    salaryBribeAmounts[u] = 
                        getRandom( maxTotalToSpend + 1 );
                    }
                else {
                    // away from home
                    salaryBribeAmounts[u] = 0;
                    }
                }
            
            
            

            // make sure amount sum is not greater than our spending cap
            int amountSum = 0;
            int i;
            for( i=0; i<6; i++ ) {
                amountSum += salaryBribeAmounts[i];
                }
            
            while( amountSum > maxTotalToSpend ) {    
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
                    
                    unit enemyUnit = inState->agentUnits[1][i-3];
                
                    for( int pu=0; pu<3; pu++ ) {
                        unit playerUnit = inState->agentUnits[0][pu];
            
                        if( playerUnit.region == enemyUnit.region ) {
                            sharingUnit = pu;
                            }
                        }

                    // [0..2] range fine for this
                    m.moveChars[ index++ ] = sharingUnit;
                    }
                else {
                    // keep old value
                    int knownValue =
                        inState->agentUnits[ i/3 ][ i%3 ].opponentBribingUnit;
                    
                    
                    if( knownValue != -1 ) {
                        
                        // turn it into [0..5] range
                        if( i < 3 ) {
                            // value describes an enemy unit that is bribing
                            // us
                            knownValue += 3;
                            }
                        }

                    m.moveChars[ index++ ] = (unsigned char)( knownValue );
                    }
                }
            
            
            
            }
            break;




        case moveUnits:
        case moveUnitsCommit: {
            // 3 chars per player unit
            // dest, bid, bribe
            m.numCharsUsed = 9;

            int dest[3];
            int bid[3];
            int bribe[3];

            int totalSpent = 0;

            int u;
            for( u=0; u<3; u++ ) {
                // start with all dests to current unit regions
                dest[u] = inState->agentUnits[0][u].region;
                }
            
            // check for collisions to be safe
            for( u=0; u<3; u++ ) {
                if( dest[u] != 0 ) {
                    for( int o=0; o<3; o++ ) {
                        if( o != u ) {
                            if( dest[o] == dest[u] ) {
                                printOut( "Units collide in chosen move!\n" );
                                }
                            }
                        }
                    }
                }



            for( u=0; u<3; u++ ) {
                
                char uniqueDestFound = false;
                
                // pick "home" half of the time for variety
                if( getRandom( 10 ) < 5 ) {
                    dest[u] = 0;
                    uniqueDestFound = true;
                    }
                

                while( !uniqueDestFound ) {    
                    dest[u] = getRandom( 7 );
                    if( dest[u] > 0 ) {
                        // add 1 to make total range [0, 2..7]
                        dest[u]++;
                        }
                    
                    if( dest == 0 ) {
                        // more than one unit can occupy home
                        uniqueDestFound = true;
                        }
                    else {
                        uniqueDestFound = true;
                        
                        // make sure this isn't same as existing dest of
                        // another unit
                        for( int o=0; o<3; o++ ) {
                            if( o != u ) {
                                if( dest[o] == dest[u] ) {
                                    // collision
                                    uniqueDestFound = false;
                                    }
                                }
                            }
                        }                    
                    }
                

                bid[u] = 0;
                if( dest[u] > 1 ) {
                    // can bid here
                    bid[u] = getRandom( inState->ourMoney.t + 1 );
                    totalSpent += bid[u];
                    }

                bribe[u] = 0;
                if( dest[u] == inState->inspectorRegion ) {
                    bribe[u] = getRandom( inState->ourMoney.t + 1 );
                    totalSpent += bribe[u];
                    }

                // cost of trip
                if( dest[u] != inState->agentUnits[0][u].region ) {
                    totalSpent++;

                    if( dest[u] == 0 ||
                        inState->agentUnits[0][u].region == 0 ) {
                        // flying to/from home costs double
                        totalSpent++;
                        }
                    }
                }
            
            // check for collisions to be safe
            for( u=0; u<3; u++ ) {
                if( dest[u] != 0 ) {
                    for( int o=0; o<3; o++ ) {
                        if( o != u ) {
                            if( dest[o] == dest[u] ) {
                                printOut( "Units collide in chosen move!\n" );
                                }
                            }
                        }
                    }
                }

            
            // pick a random spending cap
            // (otherwise, all the random choices almost always add
            //  up to equal our total money---i.e., we spend everything
            //  we have!)
            int maxTotalToSpend = getRandom( inState->ourMoney.t + 1 );



            while( totalSpent > maxTotalToSpend ) {
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
                    if( dest[u] != inState->agentUnits[0][u].region ) {
                        totalSpent --;
                        
                        if( dest[u] == 0 || 
                            inState->agentUnits[0][u].region == 0 ) {

                            // flying to/from home costs double
                            totalSpent --;
                            }

                        // stay where it is
                        dest[u] = inState->agentUnits[0][u].region;
                        }
                    } 

                
                // check for collisions that have been created by
                // this roll-back process

                char collisionFound = true;
                while( collisionFound ) {

                    collisionFound = false;
                    
                    if( dest[u] != 0 ) {

                        for( int o=0; o<3 && !collisionFound; o++ ) {
                            if( o != u ) {
                                if( dest[o] == dest[u] ) {
                                    // collision
                                    collisionFound = true;
                                    
                                    // u's dest already it's current region
                                    
                                    // roll back o to current region
                                    totalSpent--;

                                    if( dest[o] == 0 || 
                                        inState->agentUnits[0][o].region 
                                        == 0 ) {
                                        
                                        // flying to/from home costs double
                                        totalSpent --;
                                        }

                                    dest[o] = inState->agentUnits[0][o].region;
                                    
                                    
                                    // roll back bids and bribes, since
                                    // they no longer make sense w/o move
                                    totalSpent -= bid[o];
                                    totalSpent -= bribe[o];
                                    bid[o] = 0;
                                    bribe[o] = 0;
                                    
                                    // let o be the new "u", the 
                                    // just-rolled-back unit
                                    u = o;
                                    }
                                }
                            }
                        }
                    }
                }


            // check for collisions to be safe
            for( u=0; u<3; u++ ) {
                if( dest[u] != 0 ) {
                    for( int o=0; o<3; o++ ) {
                        if( o != u ) {
                            if( dest[o] == dest[u] ) {
                                printOut( "Units collide in chosen move!\n" );
                                }
                            }
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



        case moveInspector: {
            
            // 1 char, inspector dest
            m.numCharsUsed = 1;
            
            /*
            // avoid player regions
            m.moveChars[ 0 ] = getRandom( 6 ) + 2;
            */

            // pick best move here

            int maxEnemyUnit = -1;
            int maxBuyingPlusCarrying = 0;
            int maxBid = 0;
            
            for( int u=0; u<3; u++ ) {
                int diamondTotal = inState->agentUnits[1][u].diamonds;
                int bidTotal = 0;
                
                if( inState->agentUnits[1][u].diamondBid > 0 ) {
                    
                    diamondTotal += 
                        inState->regionDiamondCounts[ 
                            inState->agentUnits[1][u].region ];
                    
                    bidTotal += inState->agentUnits[1][u].diamondBid;
                    }
                
                
                if( diamondTotal >= maxBuyingPlusCarrying ) {
                    maxBuyingPlusCarrying = diamondTotal;
                    
                    if( diamondTotal == maxBuyingPlusCarrying ) {    
                        if( bidTotal > maxBid ) {
                            maxBid = bidTotal;
                            maxEnemyUnit = u;
                            }
                        }
                    else {
                        maxBid = bidTotal;
                        maxEnemyUnit = u;
                        }
                    }
                }

            if( maxEnemyUnit != -1 ) {
                // found best to attack with inspector
                m.moveChars[ 0 ] = inState->agentUnits[1][maxEnemyUnit].region;
                }
            else {
                // none to attack
                
                // find some region not occupied by us
                char occupied = true;
                int pick = -1;
                while( occupied ) {
                    occupied = false;
                    // avoid regions 1 and 2
                    pick = getRandom( 6 ) + 2;

                    for( int u=0; u<3; u++ ) {
                        if( inState->agentUnits[0][u].region == pick ) {
                            occupied = true;
                            }
                        }
                    }
                m.moveChars[ 0 ] = pick;                    
                }
            }
            break;



        case sellDiamonds:
        case sellDiamondsCommit:
            // 3 chars
            // 0 = number sold
            // 1 = image present in subsequent messages?
            // 2 = number of 300-byte messages forthcoming to contain image

            // always skip sending image
            m.numCharsUsed = 3;
            m.moveChars[0] = 
                (unsigned char)getRandom( inState->ourDiamonds + 1 );
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




int whoMovesInspector( gameState *inState ) {
    int inspectorRegion = inState->inspectorRegion;
    
    int winner = -1;
    int winningBribe = 0;
    
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            if( inState->agentUnits[p][u].region == inspectorRegion ) {
                if( inState->agentUnits[p][u].inspectorBribe > 
                    winningBribe ) {
                    
                    winner = p;
                    winningBribe = 
                        inState->agentUnits[p][u].inspectorBribe;
                    }
                }
            }
        }
    return winner;
    }



// process inspector confiscation and unit diamond collection
static void collectDiamonds( gameState *inState ) {

    int inspectorRegion = inState->inspectorRegion;
    
    // first, have him confiscate and cancel bids in his region
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            if( inState->agentUnits[p][u].region == inspectorRegion ) {
                // confiscate
                inState->agentUnits[p][u].diamonds = 0;
                
                // cancel bid
                inState->agentUnits[p][u].diamondBid = 0;
                }
            }
        }
    

    // now process remaining diamond bids
    for( int r=2; r<8; r++ ) {
        
        for( int p=0; p<2; p++ ) {
            int e = (p+1) % 2;
            
            for( int u=0; u<3; u++ ) {
                if( inState->agentUnits[p][u].region == r ) {
                    int bid = inState->agentUnits[p][u].diamondBid;
                    
                    if( bid > 0 ) {
                        int highBid = true;
                        
                        for( int i=0; i<3; i++ ) {
                            if( inState->agentUnits[e][i].region == r ) {
                                int eBid = 
                                    inState->agentUnits[e][i].diamondBid;
                                
                                if( eBid > bid ) {
                                    highBid = false;
                                    
                                    // e is winner
                            
                                    
                                    inState->agentUnits[e][i].diamonds +=
                                        inState->regionDiamondCounts[r];
                                    
                                    inState->regionDiamondCounts[r] = 0;
                                    }

                                // regardless, bid spent
                                inState->agentUnits[e][i].diamondBid = 0;
                                }
                            }
                        
                        if( highBid ) {
                            // p is winner
                            inState->agentUnits[p][u].diamonds +=
                                inState->regionDiamondCounts[r];
                            
                            inState->regionDiamondCounts[r] = 0;
                            }

                        // regardless, bid spent
                        inState->agentUnits[p][u].diamondBid = 0;
                        }
                    }
                }
            }
        }
    }

    




// pick a random value in range
static intRange collapseRange( intRange inRange ) {
    int point = inRange.lo + getRandom( ( inRange.hi - inRange.lo ) + 1 );
    
    return makeRange( point );
    }


static intRange collapseRangeToTrue( intRange inRange ) {
    return makeRange( inRange.t );
    }



gameState collapseState( gameState *inState ) {
    gameState result = *inState;
    
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
                inState->agentUnits[p][u].totalBribe.lo ) {
            
                result.agentUnits[p][u].totalBribe.hi --;
                result.agentUnits[p][u].totalBribe.lo --;
                salaryBribeSum --;
                }
            }
        else {
            if( result.agentUnits[p][u].totalSalary.lo > 
                inState->agentUnits[p][u].totalSalary.lo ) {
            
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
    
    
    /*
      // no longer need to do this

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
    */
    
    return result;
    }




gameState getMirrorState( gameState *inState ) {
    gameState result = *inState;
    
    result.ourMoney = inState->enemyMoney;
    result.ourDiamonds = inState->enemyDiamonds;
    result.enemyMoney = inState->ourMoney;
    result.enemyDiamonds = inState->ourDiamonds;

    result.knownEnemyTotalMoneyReceived = inState->knownOurTotalMoneyReceived;
    result.knownEnemyTotalMoneySpent = inState->knownOurTotalMoneySpent;

    for( int u=0; u<3; u++ ) {
        result.agentUnits[0][u] = inState->agentUnits[1][u];
        result.agentUnits[1][u] = inState->agentUnits[0][u];

        // swap home regions (both sides see 0 as their home
        for( int p=0; p<2; p++ ) {
            if( result.agentUnits[p][u].region == 0 ) {
                result.agentUnits[p][u].region = 1;
                }
            else if( result.agentUnits[p][u].region == 1 ) {
                result.agentUnits[p][u].region = 0;
                }
            }
        }
    

    return result;
    }



int playRandomGameUntilEnd( gameState inState ) {
    
    int originalMonthsLeft = inState.monthsLeft;
    NextMove originalNextMove = inState.nextMove;
    
    
    // skip sell diamonds during last month and fly everyone home
    //while( inState.monthsLeft > 0 || inState.nextMove != sellDiamonds ) {

    // stop after playing 2 months deep if we return to the same state again
    // or stop at end of game
    while( !( inState.monthsLeft == originalMonthsLeft - 2 
              && inState.nextMove == originalNextMove ) 
           &&
           ( inState.monthsLeft > 0 || inState.nextMove != sellDiamonds ) ) {
        
        
        possibleMove ourMove = getPossibleMove( &inState );
        
        gameState mirror = getMirrorState( &inState );
        possibleMove enemyMove = getPossibleMove( &mirror );
        
        if( inState.ourMoney.t < 0 || inState.enemyMoney.t < 0 ) {
            
            printOut( "Money has gone negative!\n" );
            assert( 0 );
            }
        
        // checkStateValid( inState );
        
        inState = 
            stateTransition( &inState,
                             ourMove.moveChars,
                             ourMove.numCharsUsed,
                             enemyMove.moveChars,
                             enemyMove.numCharsUsed,
                             false );
        //checkStateValid( inState );
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


    // money unit worth 1/2 of a diamond
    
    ourTotal += inState.ourMoney.t / 2;
    enemyTotal += inState.enemyMoney.t / 2;
    

    int diff = ourTotal - enemyTotal;
    
    if( diff == 0 ) {
        // diff money instead
        //printOut( "Tie at %d\n", ourTotal );
        
        // FIXME:  for testing, don't take money into account, only diamonds
        // diff = inState.ourMoney.t - inState.enemyMoney.t;
        }

    return diff;
    }



gameState stateTransition( gameState *inState, 
                           unsigned char *ourMove,
                           int ourLength,
                           unsigned char *enemyMove,
                           int enemyLength,
                           char inPreserveHiddenInfo ) {
    gameState result = *inState;
    
    switch( inState->nextMove ) {



        case salaryBribe: {
            if( ourLength != 12 && enemyLength != 12 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }

            int moneySpent[2] = { 0, 0 };
            int hiMoneyGuesses[2] = { result.ourMoney.hi, 
                                      result.enemyMoney.hi };
            int homeRegions[2] = { 0, 1 };
            
            
                    
            for( int p=0; p<2; p++ ) {
                unsigned char *pMove = ourMove;
                unsigned char *eMove = enemyMove;
                if( p == 1 ) {
                    // reversed
                    pMove = enemyMove;
                    eMove = ourMove;
                    }
                
                int e = (p+1) % 2;

                for( int u=0; u<3; u++ ) {
                    
                    // adjust true values based on actual moves

                    result.agentUnits[p][u].totalSalary.t += pMove[ u * 2 ];
                    
                    moneySpent[p] += pMove[ u * 2 ];

                    // opponent sees u unit as u+3, and describes bribe in
                    // its move there
                    int bribeAmount = eMove[ (u+3) * 2 ];
                    
                    result.agentUnits[p][u].totalBribe.t += bribeAmount;
                    
                    moneySpent[e] += bribeAmount;
                    
                    if( bribeAmount > 0 ) {
                        // change last bribing unit
                        
                        // can be -1
                        result.agentUnits[p][u].opponentBribingUnit =
                            (char)eMove[ (u+3) * 2 + 1 ];
                        }
                    
                    

                    if( !inPreserveHiddenInfo ) {
                        result.agentUnits[p][u].totalSalary = 
                            collapseRangeToTrue( 
                                result.agentUnits[p][u].totalSalary );
                    
                        result.agentUnits[p][u].totalBribe = 
                            collapseRangeToTrue( 
                                result.agentUnits[p][u].totalBribe );
                        }
                    else {
                        // increase hi estimate of range based on hi estimate
                        // of money available
                        
                        // but only if salary/bribe possible
                        int unitRegion = result.agentUnits[p][u].region;
                        
                        if( unitRegion == homeRegions[p] ) {
                            // unit at home
                            
                            // might be paid full amount of money that owner
                            // has
                            result.agentUnits[p][u].totalSalary.hi += 
                                hiMoneyGuesses[p];
                            }
                        else {
                            // unit out

                            // could be bribed?

                            // does it share a region with an enemy unit?
                            char shared = false;
                            
                            for( int i=0; i<3; i++ ) {
                                if( unitRegion == 
                                    result.agentUnits[e][i].region ) {
                                    shared = true;
                                    }
                                }
                            
                            if( shared ) {
                                // might be bribed full amount of money that
                                // opponent has

                                result.agentUnits[p][u].totalBribe.hi += 
                                    hiMoneyGuesses[e];
                                }
                            }                            
                        }
                    }
                }
            

            // track true values
            result.ourMoney.t -= moneySpent[0];
            result.enemyMoney.t -= moneySpent[1];
            
            if( result.ourMoney.t < 0 || result.enemyMoney.t < 0 ) {
                
                printOut( "Money has gone negative!\n" );
                assert( 0 );
                }


            if( ! inPreserveHiddenInfo ) {            
                result.ourMoney = collapseRangeToTrue( result.ourMoney );
                result.enemyMoney = collapseRangeToTrue( result.enemyMoney );
                }
            else {
                // could have spent no money (hi remains the same)
                // or all money
                
                // lo drops to 0
                result.ourMoney.lo = 0;
                result.enemyMoney.lo = 0;
                }
            

            result.nextMove = moveUnits;
            }
            break;





        case moveUnits: {
            if( ourLength != 9 && enemyLength != 9 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }
            result.nextMove = moveUnitsCommit;
            }
            break;




        case moveUnitsCommit: {
            if( ourLength != 9 && enemyLength != 9 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }

            // process unit moves here (switch their regions)

            unsigned char *moves[2] = { ourMove, enemyMove };
            int moneySpent[2] = { 0, 0 };
            int homeRegions[2] = { 0, 1 };

            for( int p=0; p<2; p++ ) {
                for( int u=0; u<3; u++ ) {
                    
                    // dest
                    int dest = moves[p][ u * 3 ];
                    
                    // swap home regions if this is enemy move
                    if( p == 1 && dest == 0 ) {
                        dest = 1;
                        }
                    

                    // spending on travel
                    if( dest != result.agentUnits[p][u].region ) {
                        moneySpent[p]++;

                        if( dest == homeRegions[p] ||
                            result.agentUnits[p][u].region == 
                            homeRegions[p] ) {
                            // travel to/from home costs double
                            moneySpent[p]++;
                            }
                        }
                    

                    result.agentUnits[p][u].region = dest;

                        
                    
                    // save the resulting bid/bribe for use in inspector state

                    // bid
                    int bid = moves[p][ u * 3 + 1 ];
                    result.agentUnits[p][u].diamondBid = bid;
                    moneySpent[p] += bid;
                    
                    // bribe
                    int bribe = moves[p][ u * 3 + 2 ];
                    result.agentUnits[p][u].inspectorBribe = bribe;
                    moneySpent[p] += bribe;


                    if( dest == homeRegions[p] ) {
                        // deposite any diamonds that unit is carrying
                        int diamondsCarried = result.agentUnits[p][u].diamonds;
                    
                        switch( p ) {
                            case 0:
                                result.ourDiamonds += diamondsCarried;
                                break;
                            case 1:
                                result.enemyDiamonds += diamondsCarried;
                                break;
                            }
                        result.agentUnits[p][u].diamonds = 0;
                        }
                    }
                }
            
            // spend the money that was spent
            addToRange( &( result.ourMoney ), - moneySpent[0] );
            addToRange( &( result.enemyMoney ), - moneySpent[1] );
            
            if( result.ourMoney.lo < 0 ) {
                result.ourMoney.lo = 0;
                }
            if( result.enemyMoney.lo < 0 ) {
                result.enemyMoney.lo = 0;
                }
            

            if( result.ourMoney.t < 0 || result.enemyMoney.t < 0 ) {
                
                printOut( "Money has gone negative!\n" );
                assert( 0 );
                }

            
            // sometimes, we skip move inspector state (if no one bribed him)
            
            if( whoMovesInspector( &result ) == -1 ) {
                
                collectDiamonds( &result );

                result.nextMove = sellDiamonds;
                }
            else {
                result.nextMove = moveInspector;
                }
            
            }
            break;




        case moveInspector: {
            if( ourLength != 1 && enemyLength != 1 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }
            
            
            // only pay attention to move from player that
            // actually won
            int dest = result.inspectorRegion;
            
            switch( whoMovesInspector( &result ) ) {
                case -1:
                    printOut( "Error:  AI entered moveInspector state when"
                              " no one needs to move him\n" );
                    return result;
                    break;
                case 0:
                    dest = ourMove[0];
                    break;
                case 1:
                    dest = enemyMove[0];
                    break;
                }
                    
            result.inspectorRegion = dest;
            
            // reset all bribes
            for( int p=0; p<2; p++ ) {
                for( int u=0; u<3; u++ ) {
                    result.agentUnits[p][u].inspectorBribe = 0;
                    }
                }
            

            // confiscate happens automatically in collectDiamonds
                        
                    

            // process diamond bids/collection here
            collectDiamonds( &result );
            
            result.nextMove = sellDiamonds;
            }
            break;



        case sellDiamonds: {
            if( ourLength != 3 && enemyLength != 3 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }
            result.nextMove = sellDiamondsCommit;
            }
            break;


        case sellDiamondsCommit: {
            if( ourLength != 3 && enemyLength != 3 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }
            
            result.ourDiamonds -= ourMove[0];
            result.enemyDiamonds -= enemyMove[0];
            
            int totalSold = ourMove[0] + enemyMove[0];

            int ourIncrease = 18;
            if( ourMove[0] > 0 ) {
                ourIncrease = ( ourMove[0] * 24 ) / totalSold;
                }
            addToRange( &( result.ourMoney ), ourIncrease );
            result.knownOurTotalMoneyReceived += ourIncrease;
            

            int enemyIncrease = 18;
            if( enemyMove[0] > 0 ) {
                enemyIncrease = ( enemyMove[0] * 24 ) / totalSold;
                }
            addToRange( &( result.enemyMoney ), enemyIncrease );
            result.knownEnemyTotalMoneyReceived += enemyIncrease;
            
            if( ! inPreserveHiddenInfo ) {
                result.ourMoney = collapseRangeToTrue( result.ourMoney );
                result.enemyMoney = collapseRangeToTrue( result.enemyMoney );
                }
            


            // next month
            result.monthsLeft --;
            
            accumulateDiamonds( &result );
            
            result.nextMove = salaryBribe;
            }
            break;
        }
    

    return result;            
    }



