

void initStats();
void freeStats();


void setMonthsLeft( int inMonths );
int getMonthsLeft();
void decrementMonthsLeft();


// 0 player, 1 enemy
int getPlayerMoney( int inPlayer );
int getPlayerDiamonds( int inPlayer );

void setPlayerMoney( int inPlayer, int inMoney );
void setPlayerDiamonds( int inPlayer, int inDiamonds );

void addPlayerMoney( int inPlayer, int inMoney );
void addPlayerDiamonds( int inPlayer, int inDiamonds );


// if true, opponent money is only shown if bribed unit in opponent home
void setOpponentMoneyUnknown( char inUnknown );


void showInspectorPanel( char inShow );



#define noSaleFlatRate 18
#define saleNet 24


// enables sale display
void showSale( char inShow );

void setPlayerNumToSell( int inPlayer, int inNumToSell );

// show's opponent's num to sell
void peekSale();

// reveal sale results
void finishSale();

int getPlayerEarnings( int inPlayer );
int getNumSold( int inPlayer );

// final step:  showSale( false )




// draws stats onto upper screen
void drawStats();
