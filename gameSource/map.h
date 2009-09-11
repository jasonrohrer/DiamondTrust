

// first two are home regions, plus 6 more
#define numMapRegions 8


void initMap();

void freeMap();

void drawMap();


void setSelectable( int inRegion );

int getChosenRegion( int inClickX, int inClickY );

