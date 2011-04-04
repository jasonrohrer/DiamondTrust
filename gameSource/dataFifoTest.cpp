#include <string.h>


#include "DataFifo.h"


#include "platform.h"


// provide JUST printOut from platform.h
#include <stdarg.h>

void printOut( const char *inFormatString, ... ) {
    va_list argList;
    va_start( argList, inFormatString );
    
    vprintf( inFormatString, argList );
    
    va_end( argList );
    }



static void pushString( DataFifo *inFifo, const char *inString, 
                        unsigned char inChannel ) {
    
    inFifo->addData( (unsigned char *)inString, strlen( inString ) + 1,
                     inChannel );
    }


static char fetchAndPrint( DataFifo *inFifo, char inMatchChannel,
                           unsigned char inChannel ) {

    unsigned int size;
    unsigned char *fetchData = 
        inFifo->getData( &size, 
                         inMatchChannel, inChannel );

    if( fetchData != NULL ) {
        printf( "Fetching '%s'\n", fetchData );
        delete [] fetchData;
        return true;
        }

    return false;
    }




int main() {

    DataFifo fifo;

    pushString( &fifo, "chan0 1", 0 );
    pushString( &fifo, "chan0 2", 0 );
    pushString( &fifo, "chan0 3", 0 );
    pushString( &fifo, "chan1 4", 1 );
    pushString( &fifo, "chan1 5", 1 );
    pushString( &fifo, "chan1 6", 2 );
    pushString( &fifo, "chan1 7", 1 );
    pushString( &fifo, "chan1 8", 1 );
    pushString( &fifo, "chan0 9", 0 );
    

    fifo.printFifo();    

    while( fetchAndPrint( &fifo, true, 0 ) ) {
        fifo.printFifo();
        }
    
    while( fetchAndPrint( &fifo, true, 2 ) ) {
        fifo.printFifo();
        }

    while( fetchAndPrint( &fifo, false, 0 ) ) {
        fifo.printFifo();
        }
        

    return 0;
    }

    
    
