
#include "common.h"



int roundUpToPowerOfTwo( int inValue ) {
    int bitSum = 0;
    int highBit = -1;
    
    for( int i=0; i<32; i++ ) {
        int bit = ( inValue >> i ) & 0x1;
        
        bitSum += bit;
        
        if( bit == 1 && i > highBit ) {
            highBit = i;
            }
        }
    if( bitSum == 1 ) {
        return inValue;
        }
    else {
        // flip next higher bit, and leave lower ones at 0
        return 1 << ( highBit + 1 );
        }
    
    }

