
// use common, custom random function (to avoid differences in random
// functions provided by platforms)


// as suggested by George Marsaglia
//   http://www.math.uni-bielefeld.de/~sillke/ALGORITHMS/random/marsaglia-c 
#define znew  ((z=36969*(z&65535)+(z>>16))<<16)
#define vnew  ((v=18000*(v&65535)+(v>>16))&65535)
#define IUNI  (znew+vnew)
// #define UNI   (znew+vnew)*2.328306e-10

// state defined in random.cpp
extern unsigned int z;
extern unsigned int v;


void setRandomSeed( unsigned int inA, unsigned int inB );


inline unsigned int getRandom( unsigned int inMax ) {    
    return IUNI % inMax;
    }
