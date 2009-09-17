
// first 3 are player, second 3 are enemy, last 1 is UN inspector
#define numUnits 7


void initUnits();

void freeUnits();

// draws them on map
void drawUnits();


void setUnitSelectable( int inUnit, char inSelectable );


// -1 if no unit hit
int getChosenUnit( int inClickX, int inClickY );


int getUnitRegion( int inUnit );


// draws "pending move" arrow for player units
// set back to current region to cancel move
void setUnitDestination( int inUnit, int inRegion );


// does nothing if unit not moving
void executeMove( int inUnit );


// true if latest execution done
char animationsDone();

