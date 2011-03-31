#include "measureChannel.h"
#include "platform.h"


static unsigned short allowedChannels = 0;
static unsigned short nextChannel = 0;

// FIXME:  replace with actual measurement of traffic when game runs
static int requiredTraffic = 20;
static short bestChannel = -1;
static int bestBusyRatio = 101;


void (*measureChannelCallback) (short inBestChannel);




static void measureNextChannel();



static void wmMeasureChannelCallback( void *inArg ) {
    WMMeasureChannelCallback *callbackArg = ( WMMeasureChannelCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error returned to wmMeasureChannelCallback\n" );
        measureChannelCallback( -1 );
        }
    else {
        // ignore channels too busy for our traffic
        if( callbackArg->ccaBusyRatio + requiredTraffic < 100 ) {
            if( callbackArg->ccaBusyRatio < bestBusyRatio ) {
                // best yet
                bestBusyRatio = callbackArg->ccaBusyRatio;
                bestChannel = (short) callbackArg->channel;
                }
            }
        if( nextChannel < 12 ) {
            nextChannel ++;
            measureNextChannel();
            }
        else {
            // done measuring

            if( bestChannel == -1 ) {
                measureChannelCallback( -1 );
                return;
                }
            else {
                // found one
                printOut( "Picking best channel:  %d\n", bestChannel );
                
                measureChannelCallback( bestChannel );
                return;
                }
            }
        
        }
    }



void measureNextChannel() {
    if( allowedChannels == 0 ) {
        measureChannelCallback( -1 );
        return;
        }
    while( ! ( (1 << nextChannel) & allowedChannels ) ) {
        nextChannel++;
        
        // 14 broken?
        if( nextChannel >= 13 ) {
            measureChannelCallback( -1 );
            return;
            }
        }
    
            
    
    WMErrCode result = 
        WM_MeasureChannel(
            wmMeasureChannelCallback,
            3,
            17,
            (unsigned short)( nextChannel + 1 ),
            30 );
    
    if( result != WM_ERRCODE_OPERATING ) {
        
        measureChannelCallback( -1 );
        }
    }






void startMeasureChannel( void (*inCallback) (short inBestChannel) ) {
    allowedChannels = WM_GetAllowedChannel();
    nextChannel = 0;
    bestChannel = -1;
    bestBusyRatio = 101;
    
    measureChannelCallback = inCallback;

    measureNextChannel();
    }

