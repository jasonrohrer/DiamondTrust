#include "Font.h"


void initButton();



class Button {
        

    public:
        
        // center position x,y
        Button( Font *inFont, const char *inText, int inX, int inY );
        
        // a button based on a fixed sprite, no generated text
        // inW and inH specify the true width/height of the button, which
        // may be different from the image size due to transparency
        Button( const char *inSpriteFileName, int inX, int inY,
                int inW, int inH );

        ~Button();
        

        // by default, buttons are not drawn when the game is paused
        // individual buttons (like those on the pause screen) can
        // be changed to be drawn during a pause.
        void setShowDuringPause( char inShowDuringPause );
        


        int getWidth();
        
        int getCenterX();
        int getCenterY();
        
        
        //void setVisible( char inIsVisible );
        
        
        char getPressed( int inClickX, int inClickY );

        void draw();
        
    private:
        int mFixedSprite;
        
        Font *mFont;
        char *mText;
        int mX, mY;
        int mClickRadiusX;
        int mClickRadiusY;
        

        int mW, mH;

        int mTextX, mTextY;
        char mLong;
        
        // -1 if not variable length
        int mNumMiddleParts;
        
        char mShowDuringPause;

    };

