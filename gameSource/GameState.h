#include "platform.h"


class GameState {
    public:

        GameState() {
            mStateName = "GameState";
            }
        
        
        virtual void clickState( int inX, int inY ) = 0;
        

        virtual void stepState() = 0;
        

        // draws into bottom screen
        virtual void drawState() = 0;

        
        virtual void enterStateCall() {
            printOut( "Entering GameState [%s]\n", mStateName );
            
            enterState();
            }
        
        
        // the actual enter function implemented by subclasses
        // don't call externally
        virtual void enterState() = 0;
        
        virtual char isStateDone() = 0;
        
        virtual char needsNextButton() {
            return true;
            }
        
        virtual char canStateBeBackedOut() {
            return false;
            }
        
        // empty default, so implementation not needed for states that
        // don't need it
        virtual void backOutState() {
            };
        

        virtual ~GameState() {};

        // name of state for messages
        const char *mStateName;
        
        
    };

        
