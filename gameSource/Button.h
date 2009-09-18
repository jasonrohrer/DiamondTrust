#include "Font.h"


void initButton();



class Button {
        

    public:
        
        Button( Font *inFont, char *inText, int inX, int inY );
        
        ~Button();
        
        
        //void setVisible( char inIsVisible );
        
        
        char getPressed( int inClickX, int inClickY );

        void draw();
        
    private:
        Font *mFont;
        char *mText;
        int mX, mY;
        
        int mTextX, mTextY;
        
    };

