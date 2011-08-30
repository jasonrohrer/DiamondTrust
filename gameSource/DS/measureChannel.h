


// async measures for best WiFi channel and reports to callback when done


// callback called when measuring process done
// inBestChannel param to callback will be -1 if measure fails
// inRequiredTraffic is in range [0..100], amount of traffic space needed
//  by caller (channels with less free space than this will be ignored)
void startMeasureChannel( void (*inCallback) (short inBestChannel),
                          int inRequiredTraffic );
