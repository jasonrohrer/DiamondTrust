#include "arrows.h"
#include "colors.h"
#include "minorGems/util/SimpleVector.h"


// linger on each pip for 10 frames
static int chaseSpeedFactor = 10;
//static int chaseSpeedFactor = 100;


typedef struct arrowParts {
        intPair mStart;
        intPair mEnd;
        
        int mTailSpriteID;
        int mBodySpriteID;
        int mHeadSpriteID;
        
        int mNumLineSteps;
        intPair mLineSteps[32];

        char mStartupDone;
        int mStartupProgress;

        char mDrawnThisStep;

        int mChaseProgress;
        
        
        
    } arrowParts;


static SimpleVector<arrowParts> prebuiltArrows;
static int w = 16;
static int h = 16;



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


#define stackSize 256

intPair pointStack[stackSize];

int stackTop = -1;

static char push( intPair inPair ) {
    if( stackTop < stackSize -1 ) {
        stackTop++;
        pointStack[stackTop] = inPair;
        return true;
        }

    printOut( "Stack overflow in flood fill\n" );
    return false;
    }

static char pop( intPair *outPair ) {
    if( stackTop >= 0 ) {
        *outPair = pointStack[stackTop];
        stackTop--;
        return true;
        }
    return false;
    }

static void empty() {
    stackTop = -1;
    }



static void floodFillWorker( rgbaColor *inImage, int inImageW, int inImageH,
                             intPair inStart, rgbaColor inFillColor ) {

    rgbaColor oldColor = inImage[ inStart.y * inImageW + inStart.x ];
    
    if( !push( inStart ) ) {
        return;
        }
    intPair next;
    
    while( pop( &next ) ) {
        int x = next.x;
        int y = next.y;
        
        int index = y * inImageW + x;
        
        if( equals( inImage[ index ], oldColor ) ) {
            
            inImage[ index ] = inFillColor;
        
            // add neighbors to be checked
            intPair neighbor = next;
            
            if( x - 1 >= 0 ) {
                neighbor.x = x - 1;
                if( !push( neighbor ) ) {
                    return;
                    }
                }
            if( x + 1 < inImageW ) {
                neighbor.x = x + 1;
                if( !push( neighbor ) ) {
                    return;
                    }
                }
    
            neighbor.x = next.x;
            
            if( y - 1 >= 0 ) {
                neighbor.y = y - 1;
                if( !push( neighbor ) ) {
                    return;
                    }
                }
            if( y + 1 < inImageH ) {
                neighbor.y = y + 1;
                if( !push( neighbor ) ) {
                    return;
                    }
                }
            }
        }    
    }



static void floodFill( rgbaColor *inImage, int inImageW, int inImageH,
                intPair inStart, rgbaColor inFillColor ) {
    
    // call worker
    floodFillWorker( inImage, inImageW, inImageH,
                     inStart, inFillColor );
    
    // clean up at end (only has effect in case of stack overflow)
    empty();
    }




static arrowParts *getBuiltArrow( intPair inStart, intPair inEnd ) {
    int numBuilt = prebuiltArrows.size();
    for( int i=0; i<numBuilt; i++ ) {
        arrowParts *p = prebuiltArrows.getElement( i );
        if( equals( p->mStart, inStart ) && equals( p->mEnd, inEnd ) ) {
            return p;
            }
        }
    return NULL;
    }



static char isBetween( int inX, int inA, int inB ) {
    if( inX >= inA && inX <= inB ) {
        return true;
        }
    if( inX >= inB && inX <= inA ) {
        return true;
        }
    return false;
    }



void buildArrow( intPair inStart, intPair inEnd ) {
    
    if( getBuiltArrow( inStart, inEnd ) != NULL ) {
        // already built, skip it
        return;
        }
    

    arrowParts p;
    p.mStart = inStart;
    p.mEnd = inEnd;

    p.mStartupDone = false;
    p.mStartupProgress = 0;
    p.mDrawnThisStep = false;
    p.mChaseProgress = 0;
    

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
    
    

    // scale to 1/2 image diameter
    int scaleFactor = (1 * w) / 2;
    centeredStart.x = (centeredStart.x * scaleFactor) / dist;
    centeredStart.y = (centeredStart.y * scaleFactor) / dist;
    
    centeredEnd.x = (centeredEnd.x * scaleFactor) / dist;
    centeredEnd.y = (centeredEnd.y * scaleFactor) / dist;
    

    // rotate 90
    intPair perpStart = { centeredStart.y, -centeredStart.x };
    intPair perpEnd = { centeredEnd.y, -centeredEnd.x };
    
    
            

    // adjust so it's centered at image center instead of 0,0
    centeredStart.x += w/2;
    centeredStart.y += h/2;
    centeredEnd.x += w/2;
    centeredEnd.y += h/2;
    
    perpStart.x += w/2;
    perpStart.y += h/2;
    perpEnd.x += w/2;
    perpEnd.y += h/2;
    
    

    printOut( "Arrow %d, start(%d,%d), end(%d,%d)\n",
              prebuiltArrows.size(), centeredStart.x, centeredStart.y,
              centeredEnd.x, centeredEnd.y );
    
    // now it fits in a 16x16 square image

    int stepX = centeredEnd.x - centeredStart.x;
    int stepY = centeredEnd.y - centeredStart.y;
    

    
    int xDist = intAbs( inEnd.x - inStart.x );
    int yDist = intAbs( inEnd.y - inStart.y );
    

    
    if( xDist > yDist ) {
        // horizontal-ish

        // extra pixels to avoid overlap
        if( stepX < 0 ) {
            stepX -= 4;
            }
        else {
            stepX += 4;
            }
        
        int stepIndex = 0;
        for( int x=inStart.x; 
             isBetween( x, inStart.x, inEnd.x ); 
             x+=stepX ) {
            
            int y = 
                ( ( x - inStart.x ) * (inEnd.y - inStart.y) ) 
                /
                ( inEnd.x - inStart.x ) + inStart.y;
            intPair step = { x, y };
            
            p.mLineSteps[ stepIndex ] = step;
            stepIndex ++;
            }
        p.mNumLineSteps = stepIndex;
                    
        /*
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
        */
        }
    else {
        // vertical-ish
        
        // extra pixels to avoid overlap
        if( stepY < 0 ) {
            stepY -= 4;
            }
        else {
            stepY += 4;
            }


        int stepIndex = 0;
        for( int y=inStart.y; 
             isBetween( y, inStart.y, inEnd.y ); 
             y+=stepY ) {
            
            int x = 
                ( ( y - inStart.y ) * (inEnd.x - inStart.x) ) 
                /
                ( inEnd.y - inStart.y ) + inStart.x;
            
            intPair step = { x, y };
            
            p.mLineSteps[ stepIndex ] = step;
            stepIndex ++;
            }
        p.mNumLineSteps = stepIndex;


        /*
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

        // extra border
        edgeStart.x += 1;
        edgeEnd.x += 1;

        drawLine( bodyRGBA, w, h,
                  edgeStart, edgeEnd,
                  black );
        */
        }
    
    
    //drawLine( bodyRGBA, w, h, centeredStart, centeredEnd, red );
    //drawLine( bodyRGBA, w, h, perpStart, perpEnd, red );

    // draw line tickmark

    intPair perpDelta = subtract( perpEnd, perpStart );
    
    perpDelta.x = (perpDelta.x ) / 3;
    perpDelta.y = (perpDelta.y ) / 3;
    
    
    intPair edgeAStart = add( centeredStart, perpDelta );
    intPair edgeAEnd = add( centeredEnd, perpDelta );
    
    intPair edgeBStart = subtract( centeredStart, perpDelta );
    intPair edgeBEnd = subtract( centeredEnd, perpDelta );
    
    drawLine( bodyRGBA, w, h, edgeAStart, edgeAEnd, black );
    drawLine( bodyRGBA, w, h, edgeBStart, edgeBEnd, black );
    
    drawLine( bodyRGBA, w, h, edgeAStart, edgeBStart, black );
    drawLine( bodyRGBA, w, h, edgeAEnd, edgeBEnd, black );


    /*
    // draw rotated v shape
    drawLine( bodyRGBA, w, h, centeredEnd, perpStart, black );
    drawLine( bodyRGBA, w, h, centeredEnd, perpEnd, black );

    // now back up along centered line a bit
    intPair delta;
    delta.x = ((centeredEnd.x - centeredStart.x) * 1 ) / 2;
    delta.y = ((centeredEnd.y - centeredStart.y) * 1 ) / 2;
    
    intPair newEnd;
    newEnd.x = centeredEnd.x - delta.x;
    newEnd.y = centeredEnd.y - delta.y;/

    intPair newPerpStart, newPerpEnd;
    
    newPerpStart.x = perpStart.x - delta.x;
    newPerpStart.y = perpStart.y - delta.y;
    newPerpEnd.x = perpEnd.x - delta.x;
    newPerpEnd.y = perpEnd.y - delta.y;


    //drawLine( bodyRGBA, w, h, newEnd, newPerpStart, black );
    //drawLine( bodyRGBA, w, h, newEnd, newPerpEnd, black );
    drawLine( bodyRGBA, w, h, newPerpStart, newPerpEnd, black );

    drawLine( bodyRGBA, w, h, perpStart, newPerpStart, black );
    drawLine( bodyRGBA, w, h, perpEnd, newPerpEnd, black );
    
    //drawLine( bodyRGBA, w, h, centeredStart, centeredEnd, black );
    //drawLine( bodyRGBA, w, h, perpStart, perpEnd, black );
    */


    // now fill with white
    intPair fillStart = {8,8};
    //fillStart.x = (newEnd.x + centeredEnd.x)/2;
    //fillStart.y = (newEnd.y + centeredEnd.y)/2;
    
    floodFill( bodyRGBA, w, h, fillStart, white );
    
    /*
    // grow white area
    char *tagMap = new char[ numPixels ];
    
    memset( tagMap, false, (unsigned int)numPixels );


    for( int y=1; y<h-1; y++ ) {
        for( int x=1; x<w-1; x++ ) {
                    
            int j = y * w + x;
            tagMap[j] = false;
            if( ! equals( bodyRGBA[j], white ) ) {
                int upJ = j - w;
                int downJ = j + w;
                int leftJ = j - 1;
                int rightJ = j + 1;

                if( equals( bodyRGBA[ downJ ], white ) ||
                    equals( bodyRGBA[ upJ ], white ) ||
                    equals( bodyRGBA[ leftJ ], white ) ||
                    equals( bodyRGBA[ rightJ ], white ) ) {
                    
                    // in border area
                    tagMap[j] = true;
                    }
                }
            }
        }
    for( int i=0; i<numPixels; i++ ) {
        if( tagMap[i] ) {
            bodyRGBA[i] = white;
            }
        }


    
    // grow one more time, this time in black
    memset( tagMap, false, (unsigned int)numPixels );


    for( int y=1; y<h-1; y++ ) {
        for( int x=1; x<w-1; x++ ) {
                    
            int j = y * w + x;
            tagMap[j] = false;
            if( ! equals( bodyRGBA[j], white ) ) {
                int upJ = j - w;
                int downJ = j + w;
                int leftJ = j - 1;
                int rightJ = j + 1;

                if( equals( bodyRGBA[ downJ ], white ) ||
                    equals( bodyRGBA[ upJ ], white ) ||
                    equals( bodyRGBA[ leftJ ], white ) ||
                    equals( bodyRGBA[ rightJ ], white ) ) {
                    
                    // in border area
                    tagMap[j] = true;
                    }
                }
            }
        }
    for( int i=0; i<numPixels; i++ ) {
        if( tagMap[i] ) {
            bodyRGBA[i] = black;
            }
        }

    

    delete [] tagMap;
    */


    p.mBodySpriteID = addSprite( bodyRGBA, w, h );
    delete [] bodyRGBA;

    prebuiltArrows.push_back( p );
    }



void freeArrows() {
    }




void stepArrows() {
    int numBuilt = prebuiltArrows.size();
    for( int i=0; i<numBuilt; i++ ) {
        arrowParts *p = prebuiltArrows.getElement( i );
        
        if( !p->mDrawnThisStep ) {
            // reset attributes for undrawn arrows
            p->mStartupDone = false;
            p->mStartupProgress = 0;
            p->mChaseProgress = 0;
            }
        else {
            // keep attributes, reset step flag to check again
            p->mDrawnThisStep = false;

            // step it
            if( !p->mStartupDone ) {
                p->mStartupProgress += 2;
                if( p->mStartupProgress >= p->mNumLineSteps ) {
                    p->mStartupDone = true;
                    }
                }
            /*
            else {
                // chase running
                p->mChaseProgress ++;
                
                if( p->mChaseProgress >= 
                    p->mNumLineSteps * chaseSpeedFactor ) {
                    
                    // wrap
                    p->mChaseProgress = 0;
                    }
                }
            */
            }
        }
        
    }



void drawArrow( intPair inStart, intPair inEnd, rgbaColor inColor ) {
    arrowParts *pPointer = getBuiltArrow( inStart, inEnd );
    

    
    
    if( pPointer != NULL ) {
        pPointer->mDrawnThisStep = true;
        
        arrowParts p = *pPointer;
        
        int drawLimit = p.mNumLineSteps;
        if( !p.mStartupDone ) {
            // fill them in gradually
            drawLimit = p.mStartupProgress;
            }
        
        for( int i=0; i<drawLimit; i++ ) {
            intPair step = p.mLineSteps[i];
            
            /*
            rgbaColor drawColor = inColor;

            if( p.mStartupDone ) {
            
                // chase effect in alpha
                if( p.mChaseProgress / chaseSpeedFactor == i ) {
                    // this one affected
                    
                    int factor = p.mChaseProgress % chaseSpeedFactor;
                    
                    if( factor < chaseSpeedFactor / 2 ) {
                        // fade out
                        drawColor.a = 
                            inColor.a - 
                            ( inColor.a * factor ) / ( chaseSpeedFactor / 2 );
                        }
                    else {
                        // fade back in
                        drawColor.a = 
                            ( inColor.a * (factor - chaseSpeedFactor/2) ) 
                            / ( chaseSpeedFactor / 2 );
                        }
                    
                    }
                }
            */

            drawSprite( p.mBodySpriteID, step.x - w/2, step.y - h/2, 
                        inColor );
            }
        }
    else {
        printOut( "Arrow requested to draw not found\n" );
        }
    
    }


    
