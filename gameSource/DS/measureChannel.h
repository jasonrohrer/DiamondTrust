


// async measures for best WiFi channel and reports to callback when done


// callback called when measuring process done
// inBestChannel param to callback will be -1 if measure fails
void startMeasureChannel( void (*inCallback) (short inBestChannel) );
