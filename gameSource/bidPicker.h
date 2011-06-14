

void initBidPicker();

void freeBidPicker();


void setPickerBid( int inBid );

int getPickerBid();


void drawBidPicker( int inCenterX, int inCenterY );


// returns true if it received a click
char clickBidPicker( int inX, int inY );

// just checks if it is hit by a click without actually registering the click
char bidPickerHit( int inX, int inY );



char isBidDone();
