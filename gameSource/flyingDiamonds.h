#include "common.h"



// NOTE:  for now, this only handles one diamond at a time.
// make sure flyingDiamondAnimationDone() before calling addFlyingDiamond
// again.


void initFlyingDiamonds();

void freeFlyingDiamonds();

// draws them on map
void drawFlyingDiamonds();

void stepFlyingDiamonds();

void addFlyingDiamond( intPair inStart, intPair inEnd );

char flyingDiamondAnimationDone();
