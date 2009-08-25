// interface for a game platform
// separates platform-dependent code from platform independent code
// platform-dependent code provides the main loop


// **
// functions that must be implemented by the platform
// **




// **
// functions that must be implemented by the platform-independent code
// **

// inits the game
// called by platform before entering main loop 
void gameInit();

// called during each iteration of main loop
void gameLoopTick();



