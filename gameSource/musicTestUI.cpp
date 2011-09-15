#include "music.h"
#include "Button.h"
#include "colors.h"
#include "Font.h"

#include "minorGems/util/stringUtils.h"


extern Font *font8;

extern Button *nextSongActButton;
extern Button *songRerollButton;
extern Button *switchSongButton;
extern Button *songPlusButton;
extern Button *songMinusButton;
extern Button *closeLidButton;
extern Button *openLidButton;
extern Button *muteButton;
extern Button *unmuteButton;
extern Button *volPlusButton;
extern Button *volMinusButton;


extern char soundMuted;


extern char allowManualSongActSwitching;
extern char manualLidClosed;



void clickMusicTestUI( int inX, int inY ) {

    if( allowManualSongActSwitching ) {
        if( nextSongActButton->getPressed( inX, inY ) ) {
            if( getSongAct() < 3 ) {
                nextSongAct();
                }
            else {
                backToFirstSongAct();
                }
            }
        else if( songRerollButton->getPressed( inX, inY ) ) {   
                
            char newState[12];
                
            newState[11] = '\0';
                
            for( int i=0; i<11; i++ ) {
                newState[i] = (char)( 'a' + getRandom( 26 ) );
                }
                
            setMusicState( newState );
            }
        else if( switchSongButton->getPressed( inX, inY ) ) {
            switchSongs();
            }
        else if( songPlusButton->getPressed( inX, inY ) ) {
            switchSongs( 1 );
            }
        else if( songMinusButton->getPressed( inX, inY ) ) {
            switchSongs( -1 );
            }
        else if( manualLidClosed && 
                 openLidButton->getPressed( inX, inY ) ) {
            manualLidClosed = false;
            }
        else if( ! manualLidClosed && 
                 closeLidButton->getPressed( inX, inY ) ) {
            manualLidClosed = true;
            }         
        else if( soundMuted && 
                 unmuteButton->getPressed( inX, inY ) ) {
            soundMuted = false;
            }
        else if( !soundMuted && 
                 muteButton->getPressed( inX, inY ) ) {
            soundMuted = true;
            }
        else if( volPlusButton->getPressed( inX, inY ) ) {
            setCustomSongVolume( 
                getCustomSongVolume( getCurrentSongNumber() ) + 1 );
            }
        else if( volMinusButton->getPressed( inX, inY ) ) {
            setCustomSongVolume( 
                getCustomSongVolume( getCurrentSongNumber() ) - 1 );
            }
        
        }
    
    }



void drawMusicTestUI() {
    if( allowManualSongActSwitching ) {
        nextSongActButton->draw();
        songRerollButton->draw();
        switchSongButton->draw();
        songPlusButton->draw();
        songMinusButton->draw();
            
        if( manualLidClosed ) {
            openLidButton->draw();
            }
        else {
            closeLidButton->draw();
            }    

        if( soundMuted ) {
            unmuteButton->draw();
            }
        else {
            muteButton->draw();
            }

        volPlusButton->draw();
        volMinusButton->draw();


        int labelEnd = 
            font8->drawString( "Song vol:", 2, 2, white, alignLeft );



        int currentSong = getCurrentSongNumber();
            
        for( int s=0; s<10; s++ ) {
            char *volString = autoSprintf( "%d", 
                                           getCustomSongVolume( s ) );    
            
            rgbaColor drawColor = white;
                
            if( currentSong == s ) {
                // pink
                drawColor.g = 127;
                drawColor.b = 127;
                }
    
                
            font8->drawString( volString, labelEnd + 5 + 14 + 20 * s, 
                               2, drawColor, alignRight );

            delete [] volString;
            }
            

        }
    }

