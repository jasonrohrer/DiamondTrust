#include <stdio.h>


typedef struct dataFifoElement {
        unsigned char *data;
        unsigned int numBytes;
        dataFifoElement *next;
        dataFifoElement *previous;
    } dataFifoElement;



class DataFifo {
    public:
        
        DataFifo() 
            : mHead( NULL ), mTail( NULL ){
            }

        
        ~DataFifo() {
            clearData();
            }
        

        // inData copied internally
        // added to head, returned last
        void addData( unsigned char *inData, unsigned int inNumBytes );
        
        // added to tail, returned first
        void pushData( unsigned char *inData, unsigned int inNumBytes );

        
        // gets and removes next item
        // NULL if empty
        // caller destroys
        unsigned char *getData( unsigned int *outSize );
        
        
        // peeks at copy of next item without removing
        // NULL if empty
        // caller destroys
        unsigned char *peekData( unsigned int *outSize );


        void clearData();
        

        
                    
    private:
        dataFifoElement *mHead;
        dataFifoElement *mTail;
    };
