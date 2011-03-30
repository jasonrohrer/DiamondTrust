#include "Font.h"


void initButton();



class Button {
        

    public:
        
        // center position x,y
        Button( Font *inFont, char *inText, int inX, int inY );
        
        ~Button();
        
        
        //void setVisible( char inIsVisible );
        
        
        char getPressed( int inClickX, int inClickY );

        void draw();
        
    private:
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
        
        
    };

