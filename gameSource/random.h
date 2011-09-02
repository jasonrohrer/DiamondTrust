
// use common, custom random function (to avoid differences in random
// functions provided by platforms)


// as suggested by George Marsaglia
//   http://www.math.uni-bielefeld.de/~sillke/ALGORITHMS/random/marsaglia-c 
#define znew  ((z=36969*(z&65535)+(z>>16))<<16)
#define vnew  ((v=18000*(v&65535)+(v>>16))&65535)
#define IUNI  (znew+vnew)
// #define UNI   (znew+vnew)*2.328306e-10

// state for GLOBAL rand, defined in random.cpp
extern unsigned int z;
extern unsigned int v;


void setRandomSeed( unsigned int inA, unsigned int inB );


inline unsigned int getRandom( unsigned int inMax ) {    
    return IUNI % inMax;
    }





typedef struct randState {
        unsigned int z;
        unsigned int v;
    } randState;


randState startCustomRand( unsigned int inSeedA, unsigned int inSeedB );



inline unsigned int getRandom( randState *inState, unsigned int inMax ) {
    
    
    // use local names that override globals
    // thus, these will be plugged into our IUNI macro
    unsigned int z = inState->z;
    unsigned int v = inState->v;

    unsigned int returnVal = IUNI % inMax;

    // store the new state, as modified by the macro, back in the
    // structure
    inState->z = z;
    inState->v = v;
    
    return returnVal;
    }



    
