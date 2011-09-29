#include "platform.h"

#include "minorGems/util/stringUtils.h"

class GameState {
    public:

        GameState() :
            mSubStateTransitionHappened( false ),
            mSubStateLetter( 'a' ) {

            mStateName = "GameState";
            mStateNumber = sNextStateNumber;
            sNextStateNumber++;
            }
        
        
        virtual void clickState( int inX, int inY ) = 0;
        

        virtual void stepState() = 0;
        

        // draws into bottom screen
        virtual void drawState() = 0;

        
        virtual void enterStateCall() {
            printOut( "Entering GameState [%s]\n", mStateName );
            
            resetSubState();

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
        
        virtual char isGameFullyEnded() {
            // default for all states
            // GameEndState overrides to signal very end of game-end 
            // progression
            return false;
            }
        

        virtual char canShowHelp() {
            return false;
            }
        
        virtual const char *getHelpTransKey() {
            return "";
            }
        
        

        // empty default, so implementation not needed for states that
        // don't need it
        virtual void backOutState() {
            };
        
        
        // default implementation
        // some states return parameters indicating what happened during the 
        // state
        virtual int getParameter() {
            return 0;
            }
        

        // for states that can fail due to broken connections
        virtual char isConnectionBroken() {
            // default implementation for states that don't depend on
            // connections
            return false;
            }
        


        virtual ~GameState() {};

        // name of state for messages
        const char *mStateName;


        // a unique number assigned to each instance
        // (so that each singleton state has a unique number)
        int mStateNumber;
       

        // state number plus a, b, c, d, etc. depending on sub-state 
        // advancement
        // Example:  "15b"
        // Result destroyed by caller
        virtual char *getStateShortDescription() {
            return autoSprintf( "%d%c", mStateNumber, mSubStateLetter );
            }
        
        

        // Gets the flag that is set when a transition has happened in the
        // state (so that getStateShortDescription may have changed, for
        //  example from "15b" to "15c").
        // Resets the flag to false for detecting additional transitions 
        //  later.
        virtual char getSubStateTransitionHappened() {
            char happened = mSubStateTransitionHappened;
            mSubStateTransitionHappened = false;
            return happened;
            }
        



    protected:
        
        char mSubStateTransitionHappened;
        
        char mSubStateLetter;
        
        virtual void resetSubState() {
            mSubStateLetter = 'a';
            }
        
        virtual void nextSubState() {
            // advance through ascii alphabet
            mSubStateLetter ++;
            
            if( mSubStateLetter > 'z' ) {
                mSubStateLetter = 'a';
                }

            mSubStateTransitionHappened = true;
            }
        

    private:
        static int sNextStateNumber;
        
    };

        
