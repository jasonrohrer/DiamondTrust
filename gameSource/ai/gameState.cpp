#include "ai/gameState.h"

#include "units.h"
#include "map.h"
#include "common.h"


#include <assert.h>


char *nextMoveNames[6] = { "salaryBribe", "moveUnits", "moveUnitsCommit",
                           "moveInspector", "sellDiamonds", 
                           "sellDiamondsCommit" };



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



possibleMove mutateMove( gameState *inState, possibleMove inMove,
                         unsigned int inMaxMutations ) {
    possibleMove m;

    m.numCharsUsed = inMove.numCharsUsed;
    
    // first, copy it
    memcpy( m.moveChars, inMove.moveChars, (unsigned int)m.numCharsUsed );
        

    // perform a random number of mutations
    unsigned int numMutations = getRandom( inMaxMutations ) + 1;
    
    for( unsigned int i=0; i<numMutations; i++ ) {
        
        switch( inState->nextMove ) {
            case salaryBribe: {
                
                char anyLowerable = false;
                
                for( int u=0; u<6; u++ ) {
                    if( (char)m.moveChars[u*2] > 1 ) {
                        anyLowerable = true;
                        }
                    }
                if( anyLowerable ) {
                    
                    unsigned int lowerablePick = getRandom( 6 );
                    
                    while( (char)m.moveChars[ lowerablePick * 2 ] <= 1 ) {
                        lowerablePick = getRandom( 6 );
                        }
                    
                    // lower it by 1
                    m.moveChars[lowerablePick * 2] --;
                    

                    // can also raise another by 1 w/out increasing our total
                    // money spent
                    
                    char anyRaisable = false;
                
                    for( unsigned int u=0; u<6; u++ ) {
                        if( u != lowerablePick && 
                            (char)m.moveChars[u*2] > 0 ) {
                            
                            anyRaisable = true;
                            }
                        }
                    
                    if( anyRaisable ) {
                        unsigned int raisablePick = getRandom( 6 );
                    
                        while( (char)m.moveChars[ raisablePick * 2 ] <= 0 ) {
                            raisablePick = getRandom( 6 );
                            }
                        
                        // raise it by 1
                        m.moveChars[raisablePick * 2] ++;
                        }
                    }
                }
                break;
            case moveUnits:
            case moveUnitsCommit: {
                
                // common computations shared by more than one mut type

                int countBids = 0;
                int pickA = -1;
                int pickB = -1;
                
                // pickC is unit that's bribing inspector, if any
                int pickC = -1;
                
                for( int u=0; u<3; u++ ) {
                    if( m.moveChars[ u * 3 + 1 ] > 0 ) {
                        countBids++;
                        }
                    if( m.moveChars[ u * 3 + 2 ] > 0 ) {
                        pickC = u;
                        }
                    }
                if( countBids > 1 ) {
                    // pick two to swap
                    pickA = (int)getRandom( 3 );
                    while( m.moveChars[ pickA * 3 + 1 ] == 0 ) {
                        pickA = (int)getRandom( 3 );
                        }
                    
                    pickB = (int)getRandom( 3 );
                    while( m.moveChars[ pickB * 3 + 1 ] == 0 || 
                           pickB == pickA ) {
                        pickB = (int)getRandom( 3 );
                        }
                    }

                
                unsigned int mutType = getRandom( 4 );

                switch( mutType ) {
                    case 0: {
                        if( countBids > 1 ) {
                            // swap bids between two non-zero-bid moves
                            // (total money spent the same)

                            unsigned char temp = 
                                m.moveChars[ pickA * 3 + 1 ];
                    
                            // swap
                            m.moveChars[ pickA * 3 + 1 ] =
                                m.moveChars[ pickB * 3 + 1 ];
                            m.moveChars[ pickB * 3 + 1 ] = temp;
                            }
                        }
                        
                        break;


                    case 1: {
                        if( countBids > 1 ) {
                            // lower B and potentially raise A
                            // (total money spent the same or less)

                            // only raise half the time (can't raise A w/o
                            // lowering B)
                            if( getRandom( 2 ) ) {
                                m.moveChars[ pickA * 3 + 1 ] ++;
                                }
                            
                            m.moveChars[ pickB * 3 + 1 ] --;
                            }
                        }
                        break;

                    case 2: {
                        
                        // switch bid to a different, unoccupied region
                        
                        // don't do this to units that are bribing the insp
                        // or units that aren't already switching regions
                        //   (because having them switch would cost more
                        //    money)

                        countBids = 0;
                        
                        for( int u=0; u<3; u++ ) {
                            if( m.moveChars[ u * 3 + 1 ] > 0
                                &&
                                m.moveChars[ u * 3 ] !=
                                inState->agentUnits[0][u].region
                                &&
                                m.moveChars[ u * 3 ] != 
                                inState->inspectorRegion ) {
                                
                                countBids++;
                                }
                            }

                        if( countBids > 0 ) {
                            
                            unsigned int regionPick = getRandom( 6 ) + 2;
                            
                            while( m.moveChars[ 0 ] == regionPick
                                   ||
                                   m.moveChars[ 3 ] == regionPick
                                   ||
                                   m.moveChars[ 6 ] == regionPick ) {
                                // collision
                                regionPick = getRandom( 6 ) + 2;
                                }

                            char done = false;
                            while( ! done ) {
                                unsigned int u = getRandom( 3 );
                                
                                if( m.moveChars[ u * 3 + 1 ] > 0
                                    &&
                                    m.moveChars[ u * 3 ] !=
                                    inState->agentUnits[0][u].region
                                    &&
                                    m.moveChars[ u * 3 ] != 
                                    inState->inspectorRegion ) {
                                
                                    // switch to new region
                                    m.moveChars[ u * 3 ] = 
                                        (unsigned char)regionPick;
                                    done = true;
                                    }
                                
                                }
                            }
                        
                        }
                        break;
                     
                    case 3: {
                        // modify an inspector bribe

                        if( pickC > 1 && countBids > 1 ) {
                        
                            int otherPick = pickA;
                            if( getRandom( 2 ) ) {
                                otherPick = pickB;
                                }
                            
                            if( getRandom( 2 ) ) {
                                
                                // raise other bid and lower C's bribe 
                                // (total money spent the same or less)

                                // only raise half the time (can't raise w/o
                                // lowering c's bribe)
                                if( getRandom( 2 ) ) {
                                    m.moveChars[ otherPick * 3 + 1 ] ++;
                                    }
                                m.moveChars[ pickC * 3 + 2 ] --;
                                }
                            else {
                                // lower other and raise C's bribe 
                                // (total money spent the same)
                                m.moveChars[ otherPick * 3 + 1 ] --;
                                m.moveChars[ pickC * 3 + 2 ] ++;
                                }
                            }
                        
                        }
                        break;
   
                    }
                        


                }
                break;
            default:
                break;
            }
        }
    

    return m;
    }





#define isUnitBribed( inState, inPlayer, inUnit ) \
  inState->agentUnits[inPlayer][inUnit].totalBribe.t >   \
  inState->agentUnits[inPlayer][inUnit].totalSalary.t



// was peek avaiable in our last state?
// if so, we can change our move
// if not, we have to stick with the move that we picked in the last state
static char wasPeekAvailable( gameState *inState ) {

    switch( inState->nextMove ) {
        case moveUnitsCommit: {
            
            
            for( int u=0; u<3; u++ ) {
                // have we bribed an enemy unit?
                if( isUnitBribed( inState, 1, u ) ) {
                    
                    return true;
                    }

                // is our unit bribed?
                if( isUnitBribed( inState, 0, u ) ) {
                    
                    int bribingUnit = 
                        inState->agentUnits[0][u].opponentBribingUnit;
                    
                    if( isUnitBribed( inState, 1, bribingUnit ) ) {
                        // we've bribed our briber, so we know
                        return true;
                        }

                    }
                }

            return false;
            }
            break;
        case sellDiamondsCommit: {
            
            if( inState->ourDiamonds == 0 ) {
                return false;
                }
            // do we have a bribed unit in the enemy home?
            for( int u=0; u<3; u++ ) {
                if( inState->agentUnits[1][u].region == 1 &&
                    isUnitBribed( inState, 1, u ) ) {
                    
                    return true;
                    }
                }
            // do we know that we have a bribed unit in our home?
            for( int u=0; u<3; u++ ) {
                if( inState->agentUnits[0][u].region == 0 &&
                    isUnitBribed( inState, 0, u ) ) {
                    
                    // this a bribed unit in our home, but do we know it?

                    int bribingUnit = 
                        inState->agentUnits[0][u].opponentBribingUnit;
                    
                    if( isUnitBribed( inState, 1, bribingUnit ) ) {
                        // we've bribed our briber, so we know
                        return true;
                        }
                    }
                }

            return false;
            }
            break;
        default:
            printOut( 
                "Error:  wasPeekAvailable called in a non-commit state\n" );
            return false;
        }        
    }



static int computeTripCost( gameState *inState, int inP, int inU, 
                            int inDest ) {

    int totalSpent = 0;
    
    if( inDest != inState->agentUnits[inP][inU].region ) {
        totalSpent++;
        
        if( inDest == inP ||
            inState->agentUnits[inP][inU].region == inP ) {
            // flying to/from home costs double
            totalSpent++;
            }
        }

    return totalSpent;
    }



static possibleMove getMoveUnitsMove( gameState *inState ) {
    possibleMove m;
    
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


    // first, deal with any moves that are already frozen
    for( u=0; u<3; u++ ) {
        if( inState->agentUnits[0][u].moveFrozen ) {
            
            dest[u] = inState->agentUnits[0][u].destination;
            bid[u] = inState->agentUnits[0][u].diamondBid;
            bribe[u] = inState->agentUnits[0][u].inspectorBribe;
            
            totalSpent += bid[u];
            totalSpent += bribe[u];
            
            totalSpent += computeTripCost( inState, 0, u, dest[u] );
            }
        }
    


    // pick a random spending cap, above what we have spent so far
    // (otherwise, all the random choices almost always add
    //  up to equal our total money---i.e., we spend everything
    //  we have!)
    int maxTotalToSpend = (int)getRandom( 
        (unsigned int)( inState->ourMoney.t + 1 - totalSpent ) );

    if( inState->monthsLeft == 0 ) {
        // last month, go for broke and spend it all!
        maxTotalToSpend = inState->ourMoney.t - totalSpent;
        }
    
    maxTotalToSpend += totalSpent;
    

            
    // check for collisions to be safe
    for( u=0; u<3; u++ ) {
        if( dest[u] != 0 ) {
            for( int o=0; o<3; o++ ) {
                if( o != u ) {
                    if( dest[o] == dest[u] ) {
                        printOut( "Units collide in chosen move!\n" );
                        assert( 0 );
                        }
                    }
                }
            }
        }



    for( u=0; u<3; u++ ) {
        // skip any already-set moves
        if( ! inState->agentUnits[0][u].moveFrozen ) {

            char uniqueDestFound = false;
                
            // pick "home" half of the time for variety
            if( getRandom( 10 ) < 5 ) {
                dest[u] = 0;
                uniqueDestFound = true;
                }
                

            while( !uniqueDestFound ) {    
                dest[u] = (int)getRandom( 7 );
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
                bid[u] = (int)getRandom( (unsigned int)maxTotalToSpend + 1 );
                totalSpent += bid[u];
                }

            bribe[u] = 0;
            if( dest[u] == inState->inspectorRegion ) {
                bribe[u] = (int)getRandom( (unsigned int)maxTotalToSpend + 1 );

                if( bid[u] > 0 && bribe[u] == 0 ) {
                    // illogical choice:
                    // we're bidding in inspector's region, but
                    // we're not bribing him to move him away, so
                    // our bid will be lost
                        
                    // bribe at least 1
                    bribe[u] = 1;
                    }
                    
                totalSpent += bribe[u];
                }

            // cost of trip
            totalSpent += computeTripCost( inState, 0, u, dest[u] );
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

            




    while( totalSpent > maxTotalToSpend ) {
        // spending too much
                
        // pick one to reduce
        u = (int)getRandom( 3 );
                
        // skip any that are frozen
        if( ! inState->agentUnits[0][u].moveFrozen ) {

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
    

    return m;
    }




static possibleMove getSellDiamondsMove( gameState *inState ) {
    possibleMove m;
    
    // 3 chars
    // 0 = number sold
    // 1 = image present in subsequent messages?
    // 2 = number of 300-byte messages forthcoming to contain image
    
    // always skip sending image
    m.numCharsUsed = 3;
    m.moveChars[0] = 
        (unsigned char)getRandom( (unsigned int)inState->ourDiamonds + 1 );
    m.moveChars[1] = 0;
    m.moveChars[2] = 0;
    
    return m;
    }




possibleMove getPossibleMove( gameState *inState, char inForceFreshPick ) {
    // checkStateValid( inState );
    

    possibleMove m;
    
    switch( inState->nextMove ) {



        case salaryBribe: {
            
            // 2 chars per unit (salary or bribe, plus bribing unit)
            m.numCharsUsed = 12;
            
            
            /*
            // for testing, return blank move
            for( int u=0; u<6; u++ ) {
                m.moveChars[ u* 2] = 0;
                m.moveChars[ u* 2 + 1] = (unsigned char)-1;
                }
            return m;
            */



            // pick a random spending cap
            // (otherwise, all the random choices almost always add
            //  up to equal our total money---i.e., we spend everything
            //  we have!)
            unsigned int maxTotalToSpend = 
                getRandom( (unsigned int)( inState->ourMoney.t + 1 ) );
            
            // for testing
            // int maxTotalToSpend = inState->ourMoney.t / 2;



            int salaryBribeAmounts[6];
            
            for( int u=0; u<3; u++ ) {
                unit enemyUnit = inState->agentUnits[1][u];
                
                char sharedRegion = false;

                for( int p=0; p<3; p++ ) {
                    unit playerUnit = inState->agentUnits[0][p];
            
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
                        (int)getRandom( (unsigned int)maxTotalToSpend + 1 );
                        // force bribes for testing
                        // inState->ourMoney.t;
                    }

                // player units
                if( inState->agentUnits[0][u].region == 0 ) {
                    
                    salaryBribeAmounts[u] = 
                        (int)getRandom( (unsigned int)maxTotalToSpend + 1 );
                    }
                else {
                    // away from home
                    salaryBribeAmounts[u] = 0;
                    }
                }
            
            
            

            // make sure amount sum is not greater than our spending cap
            unsigned int amountSum = 0;
            int i;
            for( i=0; i<6; i++ ) {
                amountSum += salaryBribeAmounts[i];
                }
            
            while( amountSum > maxTotalToSpend ) {    
                // pick one
                unsigned int pick = getRandom( 6 );
                
                if( salaryBribeAmounts[pick] > 0 ) {
                    // lower it
                    salaryBribeAmounts[pick] --;
                    amountSum--;
                    }
                }
            
            // assemble into a move

            for( i=0; i<6; i++ ) {
                int index = i*2;
                m.moveChars[ index++ ] = (unsigned char)salaryBribeAmounts[i];
                
                if( i > 2 && salaryBribeAmounts[i] > 0 ) {
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
                    m.moveChars[ index++ ] = (unsigned char)sharingUnit;
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
            m = getMoveUnitsMove( inState );
            /*
            {
            
            // dummy move for testing
            m.numCharsUsed = 9;
            for( int i=0; i<9; i++ ) {
                
                m.moveChars[i] = 0;
                }
            }
            */
            break;
            


        case moveUnitsCommit:
            if( inForceFreshPick || wasPeekAvailable( inState ) ) {
                // generate a fresh move, because we could peek
                m = getMoveUnitsMove( inState );
                }
            else {
                // stick with the move saved in inState

                // 3 chars per player unit
                // dest, bid, bribe
                m.numCharsUsed = 9;
                
                int dest[3];
                int bid[3];
                int bribe[3];

                int u;
                for( u=0; u<3; u++ ) {
                    dest[u] = inState->agentUnits[0][u].destination;
                    bid[u] = inState->agentUnits[0][u].diamondBid;
                    bribe[u] = inState->agentUnits[0][u].inspectorBribe;
                    }
                
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
                    bidTotal += inState->agentUnits[1][u].diamondBid;
                    }
                
                // only count diamonds for this agent if it is winning
                // the bid in that region
                
                // also, SUBTRACT the amount of diamonds that we would 
                // lose if the inspector ended up there
                
                char anyOfOurUnitsInRegion = false;
                
                for( int uu=0; uu<3; uu++ ) {
                    if( inState->agentUnits[0][uu].region ==
                        inState->agentUnits[1][u].region ) {
                        
                        anyOfOurUnitsInRegion = true;
                        
                        if( inState->agentUnits[0][uu].diamondBid <
                            inState->agentUnits[1][u].diamondBid ) {
                        
                            // they will win in this region

                            // so if we don't move inspector there,
                            // they will get the diamonds
                            diamondTotal += 
                                inState->regionDiamondCounts[ 
                                    inState->agentUnits[1][u].region ];
                            }

                        // we'd lose this much, though...
                        diamondTotal -=
                            inState->agentUnits[0][uu].diamonds;
                        }
                    }
                
                if( !anyOfOurUnitsInRegion ) {
                    // no contest there
                    // they would win those diamonds

                    // if they bid more than 0
                    if( inState->agentUnits[1][u].diamondBid > 0 ) {
                        diamondTotal += 
                            inState->regionDiamondCounts[ 
                                inState->agentUnits[1][u].region ];
                        }
                    }
                

                
                // priority to diamonds confiscated
                // break ties based on highest bid blocked
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
                m.moveChars[ 0 ] = (unsigned char)( 
                    inState->agentUnits[1][maxEnemyUnit].region );
                }
            else {
                // none to attack
                
                // find some region not occupied by us
                char occupied = true;
                int pick = 2;
                while( occupied ) {
                    occupied = false;
                    // avoid regions 1 and 2
                    pick = (int)getRandom( 6 ) + 2;

                    for( int u=0; u<3; u++ ) {
                        if( inState->agentUnits[0][u].region == pick ) {
                            occupied = true;
                            }
                        }
                    }
                m.moveChars[ 0 ] = (unsigned char)pick;                    
                }
            }
            break;



        case sellDiamonds:
            m = getSellDiamondsMove( inState );
            break;            
        case sellDiamondsCommit:
            if( inForceFreshPick || wasPeekAvailable( inState ) ) {
                // generate a fresh move, because we could peek
                m = getSellDiamondsMove( inState );
                }
            else {
                // stick with the move saved in inState

                // 3 chars
                // 0 = number sold
                // 1 = image present in subsequent messages?
                // 2 = number of 300-byte messages forthcoming to contain image
                
                // always skip sending image
                m.numCharsUsed = 3;
                m.moveChars[0] = (unsigned char)( inState->ourDiamondsToSell );
                m.moveChars[1] = 0;
                m.moveChars[2] = 0;
                }
            break;
        }


    return m;
    }





static void zeroIntArray( int *inArray, int inLength ) {
    for( int i=0; i<inLength; i++ ) {
        inArray[i] = 0;
        }
    }


// pick an index at random, with probability proportional
// to weight
static int pickRandomWeighedIndex( int *inWeights, int inLength, 
                                   unsigned int inTotalWeight ) {

    assert( inTotalWeight > 0 );
    
    
    unsigned int randomWeightPick = getRandom( inTotalWeight );
            
    unsigned int cumWeight = 0;
    int pickI = -1;
            
    for( int i=0; i<inLength && pickI == -1; i++ ) {
        cumWeight += (unsigned int)inWeights[i];
                
        if( cumWeight >= randomWeightPick ) {
            pickI = i;
            }
        }

    return pickI;
    }



// fill an array with gaussian-like weights, centered on inMean index
// returns total weight assigned
static int fauxGaussian( int *outWeights, int inLength, int inMean ) {
    
    unsigned int totalWeight = 0;
    for( int s=0; s<inLength; s++ ) {
                    
        // gaussian-like, with center around mean
        // 10 / 2**( (s - mean)**2 /10 )

        // these define, roughly, standard deviation
        // higher quotient makes the gaussian narrower
        int quotient = 1;
        int denominator = 4;
        
        int twoExponent = quotient * intPower( (s - inMean), 2 ) / denominator;
        
        if( twoExponent > 30 ) {
            // overflow
            outWeights[s] = 0;
            }
        else {
            int powerValue = intPower( 2, twoExponent );

            outWeights[s] = 10 / powerValue;
            }
        
        totalWeight += outWeights[s];
        }

    return totalWeight;
    }




static possibleMove getMoveUnitsGoodMove( gameState *inState ) {
    possibleMove m;
    
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
        bid[u] = 0;
        bribe[u] = 0;
        }


    // first, deal with any moves that are already frozen
    for( u=0; u<3; u++ ) {
        if( inState->agentUnits[0][u].moveFrozen ) {
            
            dest[u] = inState->agentUnits[0][u].destination;
            bid[u] = inState->agentUnits[0][u].diamondBid;
            bribe[u] = inState->agentUnits[0][u].inspectorBribe;
            
            totalSpent += bid[u];
            totalSpent += bribe[u];
            
            totalSpent += computeTripCost( inState, 0, u, dest[u] );
            }
        }

    // count the number of diamonds carried by agents in the field
    int atRiskDiamondCounts[2] = { 0, 0 };
    
    for( int p=0; p<2; p++ ) {
        
        for( u=0; u<3; u++ ) {
            if( u == 0 && dest[u] == 0 ) {
                // don't count these, because they're going home
                }
            else {
                atRiskDiamondCounts[p] += inState->agentUnits[p][u].diamonds;
                }
            }
        }
    

    

    // first pick a smart destination for each unit
    
    // process units in a random order
    unsigned int unitOrder[3];
    for( u=0; u<3; u++ ) {

        char assigned = false;
        while( !assigned ) {
            
            assigned = true;

            unitOrder[u] = getRandom( 3 );
            for( int uu=0; uu<u; uu++ ) {
                if( unitOrder[u] == unitOrder[uu] ) {
                    // collision with existing assignment
                    assigned = false;
                    }
                }
            }
        }
    
    int i;
    

    for( i=0; i<3; i++ ) {
        u = unitOrder[i];
        
        if( !inState->agentUnits[0][u].moveFrozen ) {
            
            int regionWeights[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            
            int totalWeight = 0;
            

            for( int r=0; r<8; r++ ) {
                if( r == 1 ) {
                    // never move into enemy home
                    regionWeights[r] = 0;
                    }
                else {
                    
                    char alreadyOccupied = false;
                    for( int uu=0; uu<3; uu++ ) {
                        if( uu!= u && dest[uu] == r ) {
                            alreadyOccupied = true;
                            }
                        }
                    
                    if( alreadyOccupied && r != 0 ) {
                        // can't move into a non-home region already
                        // being moved to by one of our agents
                        regionWeights[r] = 0;
                        }
                    else {
                        
                        // bit of weight to staying in current region
                        // because it saves money
                        if( inState->agentUnits[0][u].region == r ) {
                            regionWeights[r] +=  2;
                            }

                        // weight based on number of diamonds present
                        // approximate dollare value

                        regionWeights[r] += 
                            inState->regionDiamondCounts[r] * 4;
                        
                        if( inState->inspectorRegion == r ) {
                            // beneficial to move the inspector
                            regionWeights[r] += 4;

                            // even more beneficial depending on at-risk
                            // diamond counts
                            // (doesn't matter whoes they are... if they're
                            //  ours, we want to protect them... if they're
                            //  enemy's, we want to have them confiscated)
                            regionWeights[r] += 
                                atRiskDiamondCounts[0] + 
                                atRiskDiamondCounts[1];
                            }

                        if( r == 0 ) {
                            // should we go home?
                        
                            // if we're carrying a lot of diamonds, then yes
                            regionWeights[r] += 
                                inState->agentUnits[0][u].diamonds * 4;
                            
                            // if we're likely bribed, then yes
                            if( inState->agentUnits[0][u].totalBribe.hi >
                                inState->agentUnits[0][u].totalSalary.t ) {
                                
                                regionWeights[r] += 4;
                                }
                            
                            // if we have fewer than 2 diamonds at home,
                            // then we can't compete well in the sell-diamonds
                            // phase
                            if( inState->ourDiamonds < 2 ) {
                                // only a factor if we have diamonds
                                regionWeights[r] += 
                                    inState->agentUnits[0][u].diamonds * 4;
                                }
                            }
                        
                        }
                    }

                totalWeight += regionWeights[r];
                }
            

            // now we have weights for all the possible regions
            // (0 weights on regions that are blocked)
            dest[u] = pickRandomWeighedIndex( regionWeights, 8, totalWeight );
            
            /*
            printOut( "Region weights: " );
            for( int r=0; r<8; r++ ) {
                printOut( "%d, ", regionWeights[r] );
                }
            printOut( "\n" );
            printOut( "Picked dest %d\n", dest[u] );
            */
            }
        }
    
    
    // now we have destinations for everyone

    // if some are flying home, their diamonds are no longer at risk
    for( u=0; u<3; u++ ) {
        if( !inState->agentUnits[0][u].moveFrozen && dest[u] == 0 ) {
            
            atRiskDiamondCounts[0] += inState->agentUnits[0][u].diamonds;
            }
        }


    // how much should each one spend?
    int moneyAvailable = inState->ourMoney.t;
    // subtract money spent on frozen moves
    moneyAvailable -= totalSpent;

    // subtract trip costs
    for( u=0; u<3; u++ ) {
        if( ! inState->agentUnits[0][u].moveFrozen ) {
            int tripCost = computeTripCost( inState, 0, u, dest[u] );
            moneyAvailable -= tripCost;
            totalSpent += tripCost;
            }
        }
    
    
    
    if( moneyAvailable <= 0 ) {
        // don't even have enough for our trips

        // units had better stay where they are and save up for next time
        for( u=0; u<3; u++ ) {
            if( ! inState->agentUnits[0][u].moveFrozen ) {            
                totalSpent -= computeTripCost( inState, 0, u, dest[u] );
                dest[u] = inState->agentUnits[0][u].region;
                }
            }
        }
    else {
        
        

        // pick best amounts to spend in each region

        
        int *possibleSpendingWeights = new int[moneyAvailable + 1];
        
        int totalDiamondsInDests = 0;
        char oneDestHasInspector = false;
        int uWithInspector = -1;
        
        for( u=0; u<3; u++ ) {
            if( ! inState->agentUnits[0][u].moveFrozen ) {
                totalDiamondsInDests += 
                    inState->regionDiamondCounts[ dest[u] ];
            
                if( dest[u] == inState->inspectorRegion ) {
                    oneDestHasInspector = true;
                    uWithInspector = u;
                    }
                }
            }
        

        int idealToSpendOnInspector = 0;
        
        if( oneDestHasInspector ) {
            // spend based on how much inspector stands to take
            // vs how many diamonds we might buy
            
            idealToSpendOnInspector += 
                ( atRiskDiamondCounts[0] + 
                  atRiskDiamondCounts[1] ) / 2;
            
            idealToSpendOnInspector -= totalDiamondsInDests / 3;
            
            // bump up if there are a lot of diamonds in inspector's region
            idealToSpendOnInspector += 
                inState->regionDiamondCounts[ inState->inspectorRegion ] / 2;
            
            // bump up if unit in inspector's region is carrying a lot
            // (wanna get inspector out of there)
            idealToSpendOnInspector +=
                inState->agentUnits[0][uWithInspector].diamonds / 2;

            if( idealToSpendOnInspector > moneyAvailable ) {
                idealToSpendOnInspector = moneyAvailable;
                }
            else if( idealToSpendOnInspector < 0 ) {
                idealToSpendOnInspector = 0;
                }
            }
        
        

        for( u=0; u<3; u++ ) {
            if( ! inState->agentUnits[0][u].moveFrozen &&        
                dest[u] != 0 ) {
                
                // fraction of our money that we can spend on these diamonds
                int idealToSpendOnDiamonds = 
                    inState->regionDiamondCounts[ dest[u] ] * moneyAvailable /
                    totalDiamondsInDests;
             
                /*
                  // this doesn't make sense... how can we "try to spend
                  //   more than we have"?
                // try to spend more if we know enemy has more money than us
                idealToSpendOnDiamonds +=
                    (inState->enemyMoney.hi - inState->ourMoney.t) / 3;
                    
                */
                if( oneDestHasInspector ) {
                    // spend a bit less so that we can pay inspector
                    idealToSpendOnDiamonds -= idealToSpendOnInspector / 3;
                    }
                if( idealToSpendOnDiamonds < 0 ) {
                    idealToSpendOnDiamonds = 0;
                    }
                


                zeroIntArray( possibleSpendingWeights, moneyAvailable + 1 );
            
                unsigned int totalWeight = 
                    fauxGaussian( possibleSpendingWeights,
                                  moneyAvailable + 1, idealToSpendOnDiamonds );
                
                bid[u] = pickRandomWeighedIndex( possibleSpendingWeights,
                                                 moneyAvailable + 1,
                                                 totalWeight );
                totalSpent += bid[u];
                

                if( u == uWithInspector ) {
                    
                    // do it again for inspector

                    // but we've already computed the ideal

                    totalWeight = 
                        fauxGaussian( possibleSpendingWeights,
                                      moneyAvailable + 1, 
                                      idealToSpendOnInspector );
                    
                    bribe[u] = pickRandomWeighedIndex( possibleSpendingWeights,
                                                       moneyAvailable + 1,
                                                       totalWeight );
                    totalSpent += bribe[u];
                    }
                }
            }
        
        //printOut( "About to delete possible weights..., length %d\n",
        //          moneyAvailable + 1 );
        
        delete [] possibleSpendingWeights;        
        //printOut( "...done\n" );
        }
    

    // so now we have dest, bids, and bribes for everyone
    

    // but they may go over budget

    while( totalSpent > inState->ourMoney.t ) {
        
        // pick one to reduce
        u = (int)getRandom( 3 );
                
        // skip any that are frozen
        if( ! inState->agentUnits[0][u].moveFrozen ) {

            // priority:  trip itself (unchangeable), bribe, bid
            if( bid[u] > 0 ) {
                bid[u] --;
                totalSpent --;
                }
            else if( bribe[u] > 0 ) {
                bribe[u] --;
                totalSpent --;
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
    

    return m;
    }









static possibleMove getSellDiamondsGoodMove( gameState *inState ) {
    possibleMove m;
    
    // 3 chars
    // 0 = number sold
    // 1 = image present in subsequent messages?
    // 2 = number of 300-byte messages forthcoming to contain image
    
    // always skip sending image
    m.numCharsUsed = 3;
    
    unsigned int idealNumToSell = 0;
    
    if( inState->enemyDiamonds == 0 ) {
        idealNumToSell = 1;
        }
    else if( inState->enemyDiamonds == 1 ) {
        // switch between 1 and 2 at random
        idealNumToSell = getRandom( 2 ) + 1;
        }
    else if( inState->enemyDiamonds >= 2 ) {
        // switch between 0, 1, and 2 at random

        // going higher than this never makes sense, because even if they
        // are selling 3 or 4, 0 still beats it
        idealNumToSell = getRandom( 3 );
        }
    
    if( idealNumToSell > (unsigned int)inState->ourDiamonds ) {
        // we don't have enough to compete
        // sell 0 to be safe
        idealNumToSell = 0;
        }
    
    m.moveChars[0] = (unsigned char)idealNumToSell;
    m.moveChars[1] = 0;
    m.moveChars[2] = 0;
    
    return m;
    }







possibleMove getGoodMove( gameState *inState,
                          char inForceFreshPick ) {
    
    possibleMove m;
    
    switch( inState->nextMove ) {



        case salaryBribe: {
            
            // FIXME:  this is the same code that is in getPossibleMove

            // 2 chars per unit (salary or bribe, plus bribing unit)
            m.numCharsUsed = 12;
            
            
            /*
            // for testing, return blank move
            for( int u=0; u<6; u++ ) {
                m.moveChars[ u* 2] = 0;
                m.moveChars[ u* 2 + 1] = (unsigned char)-1;
                }
            return m;
            */



            // pick a random spending cap
            // (otherwise, all the random choices almost always add
            //  up to equal our total money---i.e., we spend everything
            //  we have!)
            unsigned int maxTotalToSpend = 
                getRandom( (unsigned int)( inState->ourMoney.t + 1 ) );
            
            // for testing
            // int maxTotalToSpend = inState->ourMoney.t / 2;



            int salaryBribeAmounts[6];
            
            for( int u=0; u<3; u++ ) {
                unit enemyUnit = inState->agentUnits[1][u];
                
                char sharedRegion = false;

                for( int p=0; p<3; p++ ) {
                    unit playerUnit = inState->agentUnits[0][p];
            
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
                        (int)getRandom( (unsigned int)maxTotalToSpend + 1 );
                        // force bribes for testing
                        // inState->ourMoney.t;
                    }

                // player units
                if( inState->agentUnits[0][u].region == 0 ) {
                    
                    salaryBribeAmounts[u] = 
                        (int)getRandom( (unsigned int)maxTotalToSpend + 1 );
                    }
                else {
                    // away from home
                    salaryBribeAmounts[u] = 0;
                    }
                }
            
            
            

            // make sure amount sum is not greater than our spending cap
            unsigned int amountSum = 0;
            int i;
            for( i=0; i<6; i++ ) {
                amountSum += salaryBribeAmounts[i];
                }
            
            while( amountSum > maxTotalToSpend ) {    
                // pick one
                unsigned int pick = getRandom( 6 );
                
                if( salaryBribeAmounts[pick] > 0 ) {
                    // lower it
                    salaryBribeAmounts[pick] --;
                    amountSum--;
                    }
                }
            
            // assemble into a move

            for( i=0; i<6; i++ ) {
                int index = i*2;
                m.moveChars[ index++ ] = (unsigned char)salaryBribeAmounts[i];
                
                if( i > 2 && salaryBribeAmounts[i] > 0 ) {
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
                    m.moveChars[ index++ ] = (unsigned char)sharingUnit;
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
            m = getMoveUnitsGoodMove( inState );
            /*
            {
            
            // dummy move for testing
            m.numCharsUsed = 9;
            for( int i=0; i<9; i++ ) {
                
                m.moveChars[i] = 0;
                }
            }
            */
            break;
            


        case moveUnitsCommit:
            if( inForceFreshPick || wasPeekAvailable( inState ) ) {
                // generate a fresh move, because we could peek
                m = getMoveUnitsGoodMove( inState );
                }
            else {
                // stick with the move saved in inState

                // 3 chars per player unit
                // dest, bid, bribe
                m.numCharsUsed = 9;
                
                int dest[3];
                int bid[3];
                int bribe[3];

                int u;
                for( u=0; u<3; u++ ) {
                    dest[u] = inState->agentUnits[0][u].destination;
                    bid[u] = inState->agentUnits[0][u].diamondBid;
                    bribe[u] = inState->agentUnits[0][u].inspectorBribe;
                    }
                
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
            
            // good alg for this already present in getPossibleMove
            // no need to replicate it here
            m = getPossibleMove( inState, inForceFreshPick );
            }
            break;



        case sellDiamonds:
            m = getSellDiamondsGoodMove( inState );
            break;            
        case sellDiamondsCommit:
            if( inForceFreshPick || wasPeekAvailable( inState ) ) {
                // generate a fresh move, because we could peek
                m = getSellDiamondsGoodMove( inState );
                }
            else {
                // stick with the move saved in inState

                // 3 chars
                // 0 = number sold
                // 1 = image present in subsequent messages?
                // 2 = number of 300-byte messages forthcoming to contain image
                
                // always skip sending image
                m.numCharsUsed = 3;
                m.moveChars[0] = (unsigned char)( inState->ourDiamondsToSell );
                m.moveChars[1] = 0;
                m.moveChars[2] = 0;
                }
            break;
        }


    return m;
    }







int getNumTotalPossibleMoves( gameState *inState ) {
    switch( inState->nextMove ) {
        case moveInspector:
            
            // in getPossibleMove, we return only the best one, if there
            // is one, or else pick at random from the equal choices

            
            // here, we might as well represent all the choices and let the
            // AIs game-run-playing algorithm pick the best one

            // can move to one of 5 other regions, or stay in current one
            //return 6;

            // actually, don't bother with this, because we know the best one
            // we're no longer running enough sim steps to be able to pick
            // the very best move here, so don't let sim do it.
            return 1;
            break;
            
            
            
        case sellDiamonds:
        case sellDiamondsCommit:

            /*
            if( inState->nextMove == sellDiamonds ||
                wasPeekAvailable( inState ) ) {
                // fresh pick
                
                // can sell between 0 and ourDiamonds diamonds
                return inState->ourDiamonds + 1;
                }
            else {
                // the move we saved in inState
                return 1;
                }
            */

            // sim doesn't work well to find a good move, because it
            // essentially computes the expected value of the move, and
            // thus almost always picks the same move (sell 0), which 
            // usually has the highest E
            
            // however, that's a dominated strategy
            // Instead, better to select a known good move, or select
            // from possibly-winning moves at random
            // Best to just pick 1 here and not sim at all.
            
            return 1;
            break;    
        
        default:
            break;
        }
    
    // other states, not practical to count
    return -1;
    }



// only call this if getNumTotalPossibleMoves did not return -1
// out moves must point to a possibleMove array with enough space for
//  getNumTotalPossibleMoves elements
void getAllPossibleMoves( gameState *inState, possibleMove *outMoves ) {
    int numMoves = getNumTotalPossibleMoves( inState );
            
    switch( inState->nextMove ) {
        case moveInspector:
            if( numMoves != 1 ) {
                printOut( "Error:  num moves doesn't match expected count "
                          " for moveInspector\n" );
                return;
                }
            
            // return the 1 very best move we can find
            outMoves[0] = getPossibleMove( inState, false );
            
            
            /*
            for( int i=0; i<numMoves; i++ ) {
                possibleMove m;
                m.numCharsUsed = 1;

                // one of the producing regions
                m.moveChars[ 0 ] = (unsigned char)( i + 2 );
                
                outMoves[i] = m;                
                }
            */
            return;
            break;

        case sellDiamonds:
        case sellDiamondsCommit:
            
            if( inState->nextMove == sellDiamonds ||
                wasPeekAvailable( inState ) ) {
        

                if( numMoves != 1 /*inState->ourDiamonds + 1*/  ) {
                    printOut( "Error:  num moves doesn't match expected count "
                              " for sellDiamonds\n" );
                    return;
                    }
            
                // return the 1 move, selected at random from good ones
                outMoves[0] = getGoodMove( inState, false );
                
                /*
                for( int i=0; i<numMoves; i++ ) {
                    possibleMove m;
                    
                    // 3 chars
                    // 0 = number sold
                    // 1 = image present in subsequent messages?
                    // 2 = number of 300-byte messages forthcoming to contain 
                    //     image
                    m.numCharsUsed = 3;
                    
                    // sell this number of diamonds
                    m.moveChars[ 0 ] = (unsigned char)i;
                    m.moveChars[1] = 0;
                    m.moveChars[2] = 0;
                    
                    outMoves[i] = m;                
                    }
                */
                }
            else {
                // just one move, the one saved in inState

                if( numMoves != 1 ) {
                    printOut( "Error:  num moves doesn't match expected count "
                              " for sellDiamonds\n" );
                    return;
                    }
                outMoves[0].numCharsUsed = 3;
                outMoves[0].moveChars[0] = 
                    (unsigned char)( inState->ourDiamondsToSell );
                outMoves[0].moveChars[1] = 0;
                outMoves[0].moveChars[2] = 0;
                }
            
            return;
            break;    
                    
        default:
            break;

        }

    printOut( 
        "Error:  getAllPossibleMoves called for an unsupported state\n" );
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

    // at most one bid per player
    int bids[2] = { 0, 0 };

    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            if( inState->agentUnits[p][u].region == inspectorRegion ) {
                if( inState->agentUnits[p][u].inspectorBribe > 0 ) {
                    
                    bids[p] = inState->agentUnits[p][u].inspectorBribe;
                    }
                }
            }
        }

    if( bids[0] > bids[1] ) {
        return 0;
        }
    else if( bids[1] > bids[0] ) {
        return 1;
        }
    else {
        return -1;
        }
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
                                
                                if( eBid >= bid ) {
                                    // e is winner (or ties us, blocking
                                    //   our bid)

                                    highBid = false;
                                    
                            

                                    if( eBid > bid ) {
                                        // e wins diamonds

                                        inState->agentUnits[e][i].diamonds +=
                                            inState->regionDiamondCounts[r];
                                        
                                        inState->regionDiamondCounts[r] = 0;
                                        }
                                    }
                                

                                // regardless, e's bid spent
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
    int point = inRange.lo + 
        (int)getRandom( (unsigned int)( 
                            ( inRange.hi - inRange.lo ) + 1 ) );
    
    return makeRange( point );
    }


static intRange collapseRangeToTrue( intRange inRange ) {
    return makeRange( inRange.t );
    }



gameState applyKnowledge( gameState *inState ) {
    gameState result = *inState;

    int moneySpentOnVisibleMoves[2] = { 0, 0 };
    

    for( int p=0; p<2; p++ ) {
        int opponent = (p + 1) % 2;
        
        for( int u=0; u<3; u++ ) {
            
            if( isUnitBribed( (&result), p, u ) ) {
                
                // salary known to opponent
                result.agentUnits[p][u].totalSalary = 
                    collapseRangeToTrue( result.agentUnits[p][u].totalSalary );

                // is the unit at home?
                if( result.agentUnits[p][u].region == p ) {
                
                    // that player's money balance is known
                    if( p == 0 ) {
                        result.ourMoney =
                            collapseRangeToTrue( result.ourMoney );
                        }
                    else {
                        result.enemyMoney =
                            collapseRangeToTrue( result.enemyMoney );
                        }
                    }
                else {
                    // can't completely collapse knowledge about money

                    // BUT... we might be able to learn something from this
                    // agents tentative move, which we can see
                    
                    // only do this in middle of move sequence
                    // or else this info has already been applied to balance
                    
                    if( inState->nextMove == moveUnitsCommit ) {
                    
                        moneySpentOnVisibleMoves[p] +=
                            inState->agentUnits[p][u].diamondBid;
                        moneySpentOnVisibleMoves[p] +=
                            inState->agentUnits[p][u].inspectorBribe;
                
                        if( inState->agentUnits[p][u].region !=
                            inState->agentUnits[p][u].destination ) {
                    
                            // agent moving, too, which costs
                            moneySpentOnVisibleMoves[p] += 1;
                    
                            if( inState->agentUnits[p][u].region == p
                                ||
                                inState->agentUnits[p][u].destination == p ) {
                        
                                // moving to/from home costs extra
                                moneySpentOnVisibleMoves[p] += 1;
                                }
                            }
                        }
                    
                    }
                
                
                // any opponent units bribed by this unit divulge their
                // bribes to the opponent
                
                for( int uu=0; uu<3; uu++ ) {
                    if( result.agentUnits[opponent][uu].opponentBribingUnit
                        == u ) {
                        result.agentUnits[opponent][uu].totalBribe
                            = collapseRangeToTrue( 
                                result.agentUnits[opponent][uu].totalBribe );
                        }
                    }
                
                }
            
            }
        }
    
    // okay, now that we've condensed salaries, bribes, and money balances,
    // we need to reconcile them together.  I.e., some high ranges may no
    // longer be accurate because of the knowledge gained (i.e., if we now
    // know opponent's money balance for sure, we might know that opponent
    // CAN'T have paid an agent as much as we thought before).
    
    for( int p=0; p<2; p++ ) {
        if( p == 0 ) {
            if( result.ourMoney.lo < moneySpentOnVisibleMoves[p] ) {
                result.ourMoney.lo = moneySpentOnVisibleMoves[p];
                }
            }
        else {
            if( result.enemyMoney.lo < moneySpentOnVisibleMoves[p] ) {
                result.enemyMoney.lo = moneySpentOnVisibleMoves[p];
                }
            }
        

        int opponent = (p + 1) % 2;

        int minTotalSpent = 0;
        int receivedTotal = 0;
        
        if( p==0 ) {
            minTotalSpent += result.knownOurTotalMoneySpent;
            receivedTotal = result.knownOurTotalMoneyReceived;
            }
        else {
            minTotalSpent += result.knownEnemyTotalMoneySpent;
            receivedTotal = result.knownEnemyTotalMoneyReceived;
            }
        
    
        for( int u=0; u<3; u++ ) {
            // money p spent paying p's agents
            minTotalSpent += result.agentUnits[p][u].totalSalary.lo;
            
            // money p spent paying opponent's agents
            minTotalSpent += result.agentUnits[opponent][u].totalBribe.lo;
            }
        
        // this is the known money gap (money they may or may not have spent)

        // gap = receivedTotal - spentTotal - knownMoneyHeld

        int moneyGap = receivedTotal - minTotalSpent;
        
        if( p==0 ) {
            moneyGap -= result.ourMoney.lo;
            }
        else {
            moneyGap -= result.enemyMoney.lo;
            }
        
        
        // (potentially) reduce hi based on this known gap
        if( p==0 ) {
            if( moneyGap < result.ourMoney.hi - result.ourMoney.lo ) {
                result.ourMoney.hi = result.ourMoney.lo + moneyGap;
                }
            }
        else {
            if( moneyGap < result.enemyMoney.hi - result.enemyMoney.lo ) {
                result.enemyMoney.hi = result.enemyMoney.lo + moneyGap;
                }
            }
            
        
        // make sure no salary or bribe gap is bigger than this known gap
        for( int u=0; u<3; u++ ) {
            if( result.agentUnits[p][u].totalSalary.hi -
                result.agentUnits[p][u].totalSalary.lo >
                moneyGap ) {
                
                result.agentUnits[p][u].totalSalary.hi =
                    result.agentUnits[p][u].totalSalary.lo + 
                    moneyGap;
                }
            if( result.agentUnits[opponent][u].totalBribe.hi -
                result.agentUnits[opponent][u].totalBribe.lo >
                moneyGap ) {
                
                result.agentUnits[opponent][u].totalBribe.hi =
                    result.agentUnits[opponent][u].totalBribe.lo + 
                    moneyGap;
                }
            }
    
    
        

        }
    
    return result;
    }





gameState collapseState( gameState *inState ) {
    gameState result = *inState;
    /*
    int moneySpentOnVisibleMoves = 0;
    
    if( inState->nextMove == moveUnitsCommit ) {
        // enemy has already specified a move

        // which parts of it can we see?
        for( int u=0; u<3; u++ ) {
            if( isUnitBribed( inState, 1, u ) ) {
                // we can see this unit's tentative move
                // so we know that enemy has at least this much money

                moneySpentOnVisibleMoves +=
                    inState->agentUnits[1][u].diamondBid;
                moneySpentOnVisibleMoves +=
                    inState->agentUnits[1][u].inspectorBribe;
                
                if( inState->agentUnits[1][u].region !=
                    inState->agentUnits[1][u].destination ) {
                    
                    // agent moving, too, which costs
                    moneySpentOnVisibleMoves += 1;
                    
                    if( inState->agentUnits[1][u].region == 1
                        ||
                        inState->agentUnits[1][u].destination == 1 ) {
                        
                        // moving to/from home costs extra
                        moneySpentOnVisibleMoves += 1;
                        }
                    }
                }
            }
        }
    */
    int u;
    
    // collapse each enemy unit salary and each player unit bribe
    
    // how much they've recieved (known)
    // - how much they've publicly spent (known)
    // - minimum that they're known to be holding 
    int availableForSalaryBribe = 
        result.knownEnemyTotalMoneyReceived -
        result.knownEnemyTotalMoneySpent -
        result.enemyMoney.lo;

    
    // subtract the money that we know enemy has spent already on 
    // salaries/bribes
    for( u=0; u<3; u++ ) {
        // enemy salary
        availableForSalaryBribe -= result.agentUnits[1][u].totalSalary.lo;

        // player unit bribes
        availableForSalaryBribe -= result.agentUnits[0][u].totalBribe.lo;
        }

    assert( availableForSalaryBribe >= 0 );
    

    // pick a random cap (or else almost all collapsed states result
    // in enemy spending all available money on salaries and bribes---
    //   we need to explore states where that's not the case)
    availableForSalaryBribe = 
        (int)getRandom( (unsigned int)availableForSalaryBribe + 1 );

    // make sure there's room under our random cap for the min values
    // that we know (random cap can get as low as zero)
    for( u=0; u<3; u++ ) {
        // enemy salary
        availableForSalaryBribe += result.agentUnits[1][u].totalSalary.lo;

        // player unit bribes
        availableForSalaryBribe += result.agentUnits[0][u].totalBribe.lo;
        }
    
    



    // sum them as we do this
    int salaryBribeSum = 0;
    
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
    

    //availableForSalaryBribe -= moneySpentOnVisibleMoves;
    

    // make sure it's not too high
    while( salaryBribeSum > availableForSalaryBribe ) {

        // pick one
        u = (int)getRandom( 3 );
        int p = (int)getRandom( 2 );
        
        // look at uncollapsed ranges for room to reduce (can't reduce below
        //  lo  value in original, uncollapsed range)

        if( p == 0 ) {
            
            if( result.agentUnits[p][u].totalBribe.lo > 
                inState->agentUnits[p][u].totalBribe.lo ) {
            
                result.agentUnits[p][u].totalBribe.hi --;
                result.agentUnits[p][u].totalBribe.lo --;
                result.agentUnits[p][u].totalBribe.t --;
                salaryBribeSum --;
                }
            }
        else {
            if( result.agentUnits[p][u].totalSalary.lo > 
                inState->agentUnits[p][u].totalSalary.lo ) {
            
                result.agentUnits[p][u].totalSalary.hi --;
                result.agentUnits[p][u].totalSalary.lo --;
                result.agentUnits[p][u].totalSalary.t --;
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
    result.ourDiamondsToSell = inState->enemyDiamondsToSell;

    result.knownOurTotalMoneyReceived = inState->knownEnemyTotalMoneyReceived;
    result.knownOurTotalMoneySpent = inState->knownEnemyTotalMoneySpent;

    
    result.enemyMoney = inState->ourMoney;
    result.enemyDiamonds = inState->ourDiamonds;
    result.enemyDiamondsToSell = inState->ourDiamondsToSell;

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

            if( result.agentUnits[p][u].destination == 0 ) {
                result.agentUnits[p][u].destination = 1;
                }
            else if( result.agentUnits[p][u].destination == 1 ) {
                result.agentUnits[p][u].destination = 0;
                }
            }
        }
    

    return result;
    }



static int getBribedCount( gameState *inState, int inPlayer ) {
    int count = 0;
    
    for( int u=0; u<3; u++ ) {
        if( inState->agentUnits[inPlayer][u].totalBribe.t >
            inState->agentUnits[inPlayer][u].totalSalary.t ) {
            count ++;
            }            
        }

    return count;
    }




int playRandomGameUntilEnd( gameState inState ) {
    
    int originalMonthsLeft = inState.monthsLeft;
    NextMove originalNextMove = inState.nextMove;
    
    if( originalNextMove == moveInspector ) {
        // this might not come up again for a while, or ever!
        originalNextMove = sellDiamonds;
        }
    

    // number of units, per side, successfully bribed
    // ticks up before each state transition if a unit is still bribed
    int ourBribedCount = 0;
    int enemyBribedCount = 0;
    
    
    // skip sell diamonds during last month and fly everyone home
    //while( inState.monthsLeft > 0 || inState.nextMove != sellDiamonds ) {

    // stop after playing 2 months deep if we return to the same state again
    // or stop at end of game
    while( !( inState.monthsLeft == originalMonthsLeft - 2 
              && inState.nextMove == originalNextMove ) 
           &&
           ( inState.monthsLeft > 0 || inState.nextMove != sellDiamonds ) ) {
            
        //possibleMove ourMove = getPossibleMove( &inState );
        possibleMove ourMove = getGoodMove( &inState );
        
        gameState mirror = getMirrorState( &inState );
        //possibleMove enemyMove = getPossibleMove( &mirror );
        possibleMove enemyMove = getGoodMove( &mirror );
        
        if( inState.ourMoney.t < 0 || inState.enemyMoney.t < 0 ) {
            
            printOut( "Money has gone negative!\n" );
            assert( 0 );
            }
        
        // checkStateValid( inState );
        
        ourBribedCount += getBribedCount( &inState, 0 );
        enemyBribedCount += getBribedCount( &inState, 1 );
        

        inState = 
            stateTransition( &inState,
                             ourMove.moveChars,
                             ourMove.numCharsUsed,
                             enemyMove.moveChars,
                             enemyMove.numCharsUsed,
                             false );
        //checkStateValid( inState );
        }
    
    // tally up everyone's diamond totals
    int ourTotal = inState.ourDiamonds;
    int enemyTotal = inState.enemyDiamonds;
    
    int ourFieldTotal = 0;
    int enemyFieldTotal = 0;
    for( int u=0; u<3; u++ ) {
        ourFieldTotal += inState.agentUnits[0][u].diamonds;
        enemyFieldTotal += inState.agentUnits[1][u].diamonds;
        }
    

    // did we play until end, or stop short?
    char gameOver = ( inState.monthsLeft == 0 && 
                      inState.nextMove == sellDiamonds );
    
    int homeDiamondsValueFactor = 10;

    // for now, have field diamonds worth 0... seems to work the best
    // (odd that even with this, AI agents never fly diamonds home, but it
    //   seems that they never need to... waste of money...
    //   instead, AI almost always controls inspector)
    int fieldDiamondValueFactor = 0;

    // diamonds in the field are worth less, since they're at risk
    
    // they're worth full value at gameOver, though, because everyone
    // flies home for free

    if( !gameOver ) {
        ourTotal *= homeDiamondsValueFactor;
        enemyTotal *= homeDiamondsValueFactor;
        
        // now add in field diamonds, which are worth a different amount
        ourTotal += fieldDiamondValueFactor * ourFieldTotal;
        enemyTotal += fieldDiamondValueFactor * enemyFieldTotal;
        }
    else {
        // add in field diamonds first, then apply factor to all
        // (because all are being flown home at game end anyway)
        ourTotal += ourFieldTotal;
        enemyTotal += enemyFieldTotal;
        
        ourTotal *= homeDiamondsValueFactor;
        enemyTotal *= homeDiamondsValueFactor;
        }


    // money unit worth (roughly) 1/4 of a diamond
        
    // according to design doc, each diamond is worth $3.2 to $5.6 
    // (depending on the amount of money injected into the game).
    // The midpoint is $4.4, which we can approximate as $4
    
    // but AI is still hoarding money by end
    // tweak this so that AI spends more...
    
    //int moneyValueFactor = 2;//6;
    
    // new:  don't value money at all here
    int moneyValueFactor = 0;
    
    
    int bribeCountFactor = 8;//200;
    

    // only diamonds matter toward victory at game end (unless there is a tie)
    if( !gameOver ) {
        // money can be used to purchase more diamonds in future rounds,
        // so it has some value

        ourTotal += inState.ourMoney.t * moneyValueFactor;
        enemyTotal += inState.enemyMoney.t * moneyValueFactor;
        }
    
    // bribed status also doesn't matter at game end... BUT
    // our bribe counts measure up until final state (which may be game end)
    // there's a benefit to bribing during those final moves

    ourTotal += enemyBribedCount * bribeCountFactor;
    enemyTotal += enemyBribedCount * bribeCountFactor;
    


    int diff = ourTotal - enemyTotal;
    
    if( gameOver && diff == 0 ) {
        // tie at end of game

        
        // winner is person with the most money

        // keep the fractional conversion rate, because winning with diamonds
        // is much "safer" than tying and relying on your money balance
        // to put you over the top
        
        diff = (inState.ourMoney.t - inState.enemyMoney.t) * moneyValueFactor;
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
            if( ourLength != 12 || enemyLength != 12 ) {
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

                            // process in a second loop below
                            }                            
                        }
                    }
                }
            



            // second loop after all payments have been made, so we know
            // which units are now bribed
            for( int p=0; p<2; p++ ) {

                int e = (p+1) % 2;

                for( int u=0; u<3; u++ ) {
                    if( inPreserveHiddenInfo ) {
                        // increase hi estimate of range based on hi estimate
                        // of money available
                        
                        // but only if bribe possible
                        int unitRegion = result.agentUnits[p][u].region;
                        
                        if( unitRegion != homeRegions[p] ) {
                            // unit out

                            // could be bribed?

                            // does it share a region with an enemy unit?
                            // only need to worry about contact with non-bribed
                            // enemy units
                            char shared = false;
                            
                            for( int i=0; i<3; i++ ) {
                                if( unitRegion == 
                                    result.agentUnits[e][i].region 
                                    &&
                                    ! isUnitBribed( (&result), e, i ) ) {
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
            if( ourLength != 9 || enemyLength != 9 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }


            // save parameters of move for when we peek
            unsigned char *moves[2] = { ourMove, enemyMove };
            
            for( int p=0; p<2; p++ ) {
                for( int u=0; u<3; u++ ) {
                    
                    // dest
                    int dest = moves[p][ u * 3 ];
                    
                    // swap home regions if this is enemy move
                    if( p == 1 && dest == 0 ) {
                        dest = 1;
                        }

                    result.agentUnits[p][u].destination = dest;

                    // bid
                    int bid = moves[p][ u * 3 + 1 ];
                    result.agentUnits[p][u].diamondBid = bid;
                    
                    // inspector bribe
                    int bribe = moves[p][ u * 3 + 2 ];
                    result.agentUnits[p][u].inspectorBribe = bribe;
                    }
                }
            


            result.nextMove = moveUnitsCommit;
            }
            break;




        case moveUnitsCommit: {
            if( ourLength != 9 || enemyLength != 9 ) {
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
                    
                    // saved destination might not equal final dest
                    // clear it
                    result.agentUnits[p][u].destination = 
                        result.agentUnits[p][u].region;
                    
                        
                    
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
           
            result.knownOurTotalMoneySpent += moneySpent[0];
            result.knownEnemyTotalMoneySpent += moneySpent[1];
            

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
            if( ourLength != 1 || enemyLength != 1 ) {
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
            if( ourLength != 3 || enemyLength != 3 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }

            result.ourDiamondsToSell = ourMove[0];
            result.enemyDiamondsToSell = enemyMove[0];


            result.nextMove = sellDiamondsCommit;
            }
            break;


        case sellDiamondsCommit: {
            if( ourLength != 3 || enemyLength != 3 ) {
                printOut( "Bad move lengths passed to AI\n" );
                return result;
                }
            
            result.ourDiamonds -= ourMove[0];
            result.enemyDiamonds -= enemyMove[0];
            
            result.ourDiamondsToSell = 0;
            result.enemyDiamondsToSell = 0;


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



