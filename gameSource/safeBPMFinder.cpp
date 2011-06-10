
#include <stdio.h>


int sampleRate = 22050;

int main() {

    int samplesPerMin = sampleRate * 60;
    

    for( int bpm=1; bpm<300; bpm++ ) {
        
        double barPerMin = bpm / 4.0;
        

        double samplesPerBar = samplesPerMin / barPerMin;
        

        if( (double)( (int)samplesPerBar ) == samplesPerBar ) {
            
            printf( "%d\n", bpm );
            
            }
        }
    


    return 0;
    }
