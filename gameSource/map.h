#include "common.h"


// first two are home regions, plus 6 more
#define numMapRegions 8


void initMap();

void freeMap();

void drawMap();


void setRegionSelectable( int inRegion, char inSelectable );


// -1 if no region hit
int getChosenRegion( int inClickX, int inClickY );


// inUnitNumber is 0-5 for player units, 6 for UN inspector
intPair getUnitPositionInRegion( int inRegion, int inUnitNumber );
