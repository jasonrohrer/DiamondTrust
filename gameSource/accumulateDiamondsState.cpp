#include "GameState.h"
#include "units.h"
#include "map.h"
#include "common.h"
#include "colors.h"

//static int activeUnit = -1;
static char stateDone = false;

extern const char *statusMessage;
extern const char *statusSubMessage;


extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;
extern unsigned char satelliteFade;

extern int titleSpriteID;



class AccumulateDiamondsState : public GameState {
    public:
        
        AccumulateDiamondsState() {
            mStateName = "AccumulateDiamondsState";
            }
        

        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        

        // destructor?
        //virtual ~GameState();
        
    };








void AccumulateDiamondsState::clickState( int inX, int inY ) {
    
    // no clicking!

    // avoid unused variable warnings
    inX = inY;
    }


static int stepsSinceDiamondTick = 0;
static int minDiamondSteps = 32;


void AccumulateDiamondsState::stepState() {

    stepsSinceDiamondTick++;
        
    stepDiamondBorderFade();

    if( stepsSinceDiamondTick > minDiamondSteps ) {
        stepsSinceDiamondTick = 0;
        char done =  accumulateDiamondsStep();
        
        
        if( done ) {
            stateDone = true;
            }
        }

    if( satelliteFade > 0 ) {
        
        if( satelliteFade >= 4 ) {
            satelliteFade -= 4;
            }
        else {
            satelliteFade = 0;
            }
        //printOut( "Fade = %d\n", satelliteFade );
        
        }
    
        
    }



void AccumulateDiamondsState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    startNewSpriteLayer();
    
    if( satelliteFade > 0 ) {
        
        // draw fading satellite map on top
        rgbaColor satelliteColor = white;
        satelliteColor.a = satelliteFade;
        
        drawSprite( satelliteTopSpriteID, 
                    0,0, satelliteColor );
        drawSprite( satelliteBottomSpriteID, 
                    0,satelliteBottomHalfOffset, satelliteColor );
        }
    }






void AccumulateDiamondsState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_accumulate" );    
    statusSubMessage = translate( "phaseSubStatus_accumulate" );    
    
    showUnitMoves( false );
    
    showAllUnitMoves( false );

    accumulateDiamondsStart();
    }






// singleton
static AccumulateDiamondsState state;


GameState *accumulateDiamondsState = &state;
