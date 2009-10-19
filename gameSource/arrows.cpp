#include "arrows.h"
#include "colors.h"
#include "minorGems/util/SimpleVector.h"


typedef struct arrowParts {
        intPair mStart;
        intPair mEnd;
        
        int mTailSpriteID;
        int mBodySpriteID;
        int mHeadSpriteID;

        int mStepX;
        int mStepY;
        
    } arrowParts;


static SimpleVector<arrowParts> prebuiltArrows;
static int w = 32;
static int h = 32;



void initArrows() {
    }


void swap( int *inA, int *inB ) {
    int temp = *inA;
    *inA = *inB;
    *inB = temp;
    }


// Bresenham's integer line algorithm
// found in wikipedia
void drawLine( rgbaColor *inImage, int inImageW, int inImageH,
               intPair inStart, intPair inEnd,
               rgbaColor inLineColor ) {
    
    int x0 = inStart.x;
    int y0 = inStart.y;
    int x1 = inEnd.x;
    int y1 = inEnd.y;
    

    char steep = intAbs(y1 - y0) > intAbs(x1 - x0);
    
    if( steep ) {
        swap( &x0, &y0 );
        swap( &x1, &y1 );
        }
    if( x0 > x1 ) {
        swap( &x0, &x1 );
        swap( &y0, &y1 );
        }
    
    int deltax = x1 - x0;
    
    int deltay = intAbs( y1 - y0 );
    
    int error = deltax / 2;
    
    int ystep;
    
    int y = y0;
    
    if( y0 < y1 ) {
        ystep = 1;
        }
    else {
        ystep = -1;
        }
    
    for( int x=x0; x<=x1; x++ ) {
        int plotX, plotY;
        
        if( steep ) {
            plotX = y;
            plotY = x;
            }
        else {
            plotX = x;
            plotY = y;
            }
        
        inImage[ plotY * inImageW + plotX ] = inLineColor;
        
        error = error - deltay;
        
        if( error < 0 ) {
            y += ystep;
            
            error += deltax;
            }
        }
    }





void buildArrow( intPair inStart, intPair inEnd ) {
    
    arrowParts p;
    p.mStart = inStart;
    p.mEnd = inEnd;


    int numPixels = w * h;
    
    rgbaColor *bodyRGBA = new rgbaColor[ w * h ];
    
    // fill with transparent
    for( int i=0; i<numPixels; i++ ) {
        bodyRGBA[i] = transparent;
        }
    

    // shrink start/end down so they fit in image, centered, with radius of 8
    
    

    // first center them around 0,0
    
    int dist = intDistance( inStart, inEnd );
    

    intPair middle = { (inStart.x + inEnd.x) / 2,
                       (inStart.y + inEnd.y) / 2 };

    
    intPair centeredStart = inStart;
    intPair centeredEnd = inEnd;
    
    centeredStart.x -= middle.x;
    centeredStart.y -= middle.y;
    
    centeredEnd.x -= middle.x;
    centeredEnd.y -= middle.y;
    
    

    // scale by 3/4 image diameter
    int scaleFactor = (3 * w ) / 4;
    centeredStart.x = (centeredStart.x * scaleFactor) / dist;
    centeredStart.y = (centeredStart.y * scaleFactor) / dist;
    
    centeredEnd.x = (centeredEnd.x * scaleFactor) / dist;
    centeredEnd.y = (centeredEnd.y * scaleFactor) / dist;
    
    // adjust so it's centered at image center instead of 0,0
    centeredStart.x += w/2;
    centeredStart.y += h/2;
    centeredEnd.x += w/2;
    centeredEnd.y += h/2;
    
    printOut( "Arrow %d, start(%d,%d), end(%d,%d)\n",
              prebuiltArrows.size(), centeredStart.x, centeredStart.y,
              centeredEnd.x, centeredEnd.y );
    
    // now it fits in a 16x16 square image

    p.mStepX = centeredEnd.x - centeredStart.x;
    p.mStepY = centeredEnd.y - centeredStart.y;
    

    
    int xDist = intAbs( inEnd.x - inStart.x );
    int yDist = intAbs( inEnd.y - inStart.y );
    

    
    if( xDist > yDist ) {
        // horizontal-ish
        
        // edges above and below

        intPair edgeStart = centeredStart;
        intPair edgeEnd = centeredEnd;
        edgeStart.y -= 3;
        edgeEnd.y -= 3;
        
                
        drawLine( bodyRGBA, w, h,
                  edgeStart, edgeEnd,
                  black );
        
        edgeStart.y += 6;
        edgeEnd.y += 6;

        drawLine( bodyRGBA, w, h,
                  edgeStart, edgeEnd,
                  black );

        // fill vertically between edges
        for( int x=0; x<w; x++ ) {
            char fillStarted = false;
            char fillDone = false;
            for( int y=0; y<h && !fillDone; y++ ) {
                if( !fillStarted ) {
                    if( equals( bodyRGBA[ y * w + x ], black ) ) {
                        fillStarted = true;
                        }
                    }
                else {
                    if( equals( bodyRGBA[ y * w + x ], black ) ) {
                        fillDone = true;
                        }
                    else {
                        // middle, fill it
                        bodyRGBA[ y * w + x ] = white;
                        }
                    }
                }
            }
        }
    else {
        // vertical-ish
        

        // edges to left and right

        intPair edgeStart = centeredStart;
        intPair edgeEnd = centeredEnd;
        edgeStart.x -= 3;
        edgeEnd.x -= 3;
        
        drawLine( bodyRGBA, w, h,
                  edgeStart, edgeEnd,
                  black );
        
        edgeStart.x += 6;
        edgeEnd.x += 6;

        drawLine( bodyRGBA, w, h,
                  edgeStart, edgeEnd,
                  black );

        // fill horizontally between edges
        for( int y=0; y<h; y++ ) {
            char fillStarted = false;
            char fillDone = false;
            for( int x=0; x<w && !fillDone; x++ ) {
                if( !fillStarted ) {
                    if( equals( bodyRGBA[ y * w + x ], black ) ) {
                        fillStarted = true;
                        }
                    }
                else {
                    if( equals( bodyRGBA[ y * w + x ], black ) ) {
                        fillDone = true;
                        }
                    else {
                        // middle, fill it
                        bodyRGBA[ y * w + x ] = white;
                        }
                    }
                }
            }
        }
    

    //drawLine( bodyRGBA, w, h, centeredStart, centeredEnd, red );

    p.mBodySpriteID = addSprite( bodyRGBA, w, h );
    delete [] bodyRGBA;

    prebuiltArrows.push_back( p );
    }



void freeArrows() {
    }





void drawArrow( intPair inStart, intPair inEnd, rgbaColor inColor ) {
    int numBuilt = prebuiltArrows.size();
    arrowParts p;
    char found = false;
    for( int i=0; i<numBuilt && !found; i++ ) {
        p = *( prebuiltArrows.getElement( i ) );
        if( equals( p.mStart, inStart ) && equals( p.mEnd, inEnd ) ) {
            found = true;
            }
        }
    
    if( found ) {
        

        intPair current = inStart;
        
        while( intAbs( current.x - inStart.x ) < 
               intAbs( inEnd.x - inStart.x ) 
               &&
               intAbs( current.y - inStart.y ) < 
               intAbs( inEnd.y - inStart.y ) ) {
            
            drawSprite( p.mBodySpriteID, 
                        current.x - w/2, current.y -  h/2, 
                        inColor );
            current.x += p.mStepX;
            current.y += p.mStepY;
            }
        }
    else {
        printOut( "Arrow requested to draw not found\n" );
        }
    
    }


    
