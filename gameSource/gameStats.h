

void initStats();
void freeStats();


// 0 player, 1 enemy
int getPlayerMoney( int inPlayer );
int getPlayerDiamonds( int inPlayer );

void setPlayerMoney( int inPlayer, int inMoney );
void setPlayerDiamonds( int inPlayer, int inDiamonds );

void addPlayerMoney( int inPlayer, int inMoney );
void addPlayerDiamonds( int inPlayer, int inDiamonds );



// draws stats onto upper screen
void drawStats();
