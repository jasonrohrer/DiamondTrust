#include "DataFifo.h"


#include <string.h>


void DataFifo::addData( unsigned char *inData, unsigned int inNumBytes ) {
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
            
    f->data = new unsigned char[ inNumBytes ];
            
    memcpy( f->data, inData, inNumBytes );
    }



void DataFifo::pushData( unsigned char *inData, unsigned int inNumBytes ) {
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
            
    f->data = new unsigned char[ inNumBytes ];
            
    memcpy( f->data, inData, inNumBytes );
    }


unsigned char *DataFifo::getData( unsigned int *outSize ) {
    if( mTail == NULL ) {
        return NULL;
        }
    else {
        unsigned char *returnData = mTail->data;
        *outSize = mTail->numBytes;

        dataFifoElement *oldTail = mTail;
        mTail = mTail->previous;
                
        if( mTail == NULL ) {
            mHead = NULL;
            }
        else {
            mTail->next = NULL;
            }
        delete oldTail;
                
        return returnData;
        }
    }



unsigned char *DataFifo::peekData( unsigned int *outSize ) {
    if( mTail == NULL ) {
        return NULL;
        }
    else {
        unsigned int numBytes = mTail->numBytes;
        
        unsigned char *returnData = new unsigned char[ numBytes ];
        memcpy( returnData, mTail->data, numBytes );
        *outSize = mTail->numBytes;
        
        return returnData;
        }
    }



void DataFifo::clearData() {
    unsigned int numBytes;
    unsigned char *data = getData( &numBytes );
            
            
    while( data != NULL ) {
        delete [] data;
        data = getData( &numBytes );
        }
    }
