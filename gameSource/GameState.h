

class GameState {
    public:
        
        virtual void clickState( int inX, int inY ) = 0;
        

        virtual void stepState() = 0;
        

        // draws into bottom screen
        virtual void drawState() = 0;


        virtual void enterState() = 0;
        
        virtual char isStateDone() = 0;
        

        virtual ~GameState() {};
        
    };

        
