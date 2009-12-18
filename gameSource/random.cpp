#include "random.h"

unsigned int z=362436069, v=521288629;


void setRandomSeed( unsigned int inA, unsigned int inB ) {
    z = inA; 
    v = inB;
    }
