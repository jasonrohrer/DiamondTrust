
// first 3 are player, second 3 are enemy, last 1 is UN inspector
#define numUnits 7


void initUnits();

void freeUnits();

// draws them on map
void drawUnits();

void stepUnits();


void setUnitSelectable( int inUnit, char inSelectable );

void setAllUnitsNotSelectable();

void setPlayerUnitsSelectable( char inSelectable );


// -1 to set none
void setActiveUnit( int inUnit );

int getActiveUnit();



// -1 if no unit hit
int getChosenUnit( int inClickX, int inClickY );


int getUnitRegion( int inUnit );


// draws "pending move" arrow for player units
// set back to current region to cancel move
void setUnitDestination( int inUnit, int inRegion );

// returns unit's current region if unit has no destination
int getUnitDestination( int inUnit );



// does nothing if unit not moving to a new region
void executeMove( int inUnit );


// true if latest execution done
char animationsDone();

