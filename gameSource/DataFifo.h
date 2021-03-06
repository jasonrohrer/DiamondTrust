#include <stdio.h>


typedef struct dataFifoElement {
        unsigned char *data;
        unsigned int numBytes;
        unsigned char channel;
        dataFifoElement *next;
        dataFifoElement *previous;
        char lastMessageFlag;
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
        void addData( unsigned char *inData, unsigned int inNumBytes,
                      unsigned char inChannel, 
                      char inLastMessageFlag = false );
        
        // added to tail, returned first
        void pushData( unsigned char *inData, unsigned int inNumBytes,
                       unsigned char inChannel, 
                       char inLastMessageFlag = false );

        
        // gets and removes next item
        // NULL if empty
        // caller destroys
        unsigned char *getData( unsigned int *outSize, 
                                char inMatchChannel, unsigned char inChannel,
                                unsigned char *outChannel,
                                char *outLastMessageFlag = NULL );
        
        
        // peeks at copy of next item without removing
        // NULL if empty
        // caller destroys
        unsigned char *peekData( unsigned int *outSize, 
                                 char inMatchChannel, 
                                 unsigned char inChannel,
                                 unsigned char *outChannel,
                                 char *outLastMessageFlag = NULL  );


        void clearData();
        


        void printFifo();
        
                    
    private:

        dataFifoElement *findMatch( char inMatchChannel, 
                                    unsigned char inChannel );
        

        dataFifoElement *mHead;
        dataFifoElement *mTail;
    };
