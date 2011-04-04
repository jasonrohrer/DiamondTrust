#include "DataFifo.h"


#include <string.h>

// for printOut definition
#include "platform.h"


void DataFifo::addData( unsigned char *inData, unsigned int inNumBytes,
                        unsigned char inChannel ) {
    dataFifoElement *f = new dataFifoElement;
            
    f->next = mHead;
    f->previous = NULL;
            
    if( mHead != NULL ) {
        mHead->previous = f;
        }
    else{
        //empty
        mTail = f;
        }

    mHead = f;
    f->numBytes = inNumBytes;
    f->channel = inChannel;
    
    f->data = new unsigned char[ inNumBytes ];
            
    memcpy( f->data, inData, inNumBytes );
    }



void DataFifo::pushData( unsigned char *inData, unsigned int inNumBytes,
                         unsigned char inChannel ) {
    dataFifoElement *f = new dataFifoElement;
            
    f->next = NULL;
    f->previous = mTail;
            
    if( mTail != NULL ) {
        mTail->next = f;
        }
    else{
        //empty
        mHead = f;
        }

    mTail = f;
    f->numBytes = inNumBytes;
    f->channel = inChannel;
    
    f->data = new unsigned char[ inNumBytes ];
            
    memcpy( f->data, inData, inNumBytes );
    }



dataFifoElement *DataFifo::findMatch( char inMatchChannel,
                                      unsigned char inChannel ) {
    dataFifoElement *match = mTail;
    
    if( ! inMatchChannel ) {
        return match;
        }
    
    // else search for channel match

    while( match != NULL &&
           match->channel != inChannel ) {
        match = match->previous;
        }
    
    return match;
    }



unsigned char *DataFifo::getData( unsigned int *outSize,
                                  char inMatchChannel,
                                  unsigned char inChannel,
                                  unsigned char *outChannel ) {
  
    
    dataFifoElement *match = findMatch( inMatchChannel, inChannel );
    
    if( match == NULL ) {
        return NULL;
        }
    


    unsigned char *returnData = match->data;
    *outSize = match->numBytes;
    *outChannel = match->channel;
    
    if( match == mHead || match == mTail ) {    
        if( match == mHead ) {
            // new head
            mHead = match->next;
            }
        if( match == mTail ) {
            // new tail
            mTail = match->previous;
            }
        }
    
    if( match->previous != NULL ) {
        match->previous->next = match->next;
        }
    
    if( match->next != NULL ) {
        match->next->previous = match->previous;
        }
        
    delete match;
    
    return returnData;
    }



unsigned char *DataFifo::peekData( unsigned int *outSize,
                                   char inMatchChannel,
                                   unsigned char inChannel,
                                   unsigned char *outChannel ) {

    dataFifoElement *match = findMatch( inMatchChannel, inChannel );

    if( match == NULL ) {
        return NULL;
        }


    if( mTail == NULL ) {
        return NULL;
        }


    unsigned int numBytes = match->numBytes;

    unsigned char *returnData = new unsigned char[ numBytes ];
    memcpy( returnData, match->data, numBytes );
    *outSize = match->numBytes;
    *outChannel = match->channel;
    
    return returnData;
    }



void DataFifo::clearData() {
    unsigned int numBytes;
    unsigned char channel;
    unsigned char *data = getData( &numBytes, false, 0, &channel );
            
            
    while( data != NULL ) {
        delete [] data;
        data = getData( &numBytes, false, 0, &channel );
        }
    }



void DataFifo::printFifo() {
    printOut( "Fifo:\n" );
    
    dataFifoElement *next = mHead;
    
    int index = 0;

    while( next != NULL ) {
        
        if( index > 25 ) {
            printOut( "Cutting off print-out\n" );
            break;
            }

        printOut( "  %d:  [%X], chan %d, %d bytes, '%10s'\n",
                index, (unsigned int)next, 
                next->channel, next->numBytes, next->data );
        
        index ++;

        next = next->next;
        }
    }
