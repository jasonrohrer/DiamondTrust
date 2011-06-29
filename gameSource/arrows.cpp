#include "arrows.h"
#include "colors.h"
#include "minorGems/util/SimpleVector.h"


// linger on each pip for 10 frames
//static int chaseSpeedFactor = 10;
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


static void swap( int *inA, int *inB ) {
    int temp = *inA;
    *inA = *inB;
    *inB = temp;
    }


// Bresenham's integer line algorithm
// found in wikipedia
static void drawLine( rgbaColor *inImage, int inImageW,
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
    
    

    // scale to 1/2 image diameter, target length of w/2
    int targetLength = w/2;
    intPair unscaledStart = centeredStart;
    intPair unscaledEnd = centeredEnd;
    
    int extra = 0;
    
    int tickLength = 0;

    while( tickLength < targetLength ) {
        
        int scaleFactor = ( (10+extra) * w) / 20;
        
        centeredStart.x = (unscaledStart.x * scaleFactor) / dist;
        centeredStart.y = (unscaledStart.y * scaleFactor) / dist;
    
        centeredEnd.x = (unscaledEnd.x * scaleFactor) / dist;
        centeredEnd.y = (unscaledEnd.y * scaleFactor) / dist;
    
        tickLength = intDistance( centeredStart, centeredEnd );
        
        extra++;
        }
    
    

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
    
    

    /*printOut( "Arrow %d, start(%d,%d), end(%d,%d)\n",
              prebuiltArrows.size(), centeredStart.x, centeredStart.y,
              centeredEnd.x, centeredEnd.y );
    */

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
        
        
        int xDelta = intAbs( inEnd.x - inStart.x );
        // round step size up to allow exact filling of span
        int numSteps = xDelta / stepX;
        stepX = xDelta / numSteps;
        

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

        
        int yDelta = intAbs( inEnd.y - inStart.y );
        // round step size up to allow exact filling of span
        int numSteps = yDelta / stepY;
        stepY = yDelta / numSteps;
        

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
        }
    
    
    // draw line tickmark

    intPair perpDelta = subtract( perpEnd, perpStart );
    
    perpDelta.x = (perpDelta.x ) / 3;
    perpDelta.y = (perpDelta.y ) / 3;

    // constrain all tick widths within a range
    // target radius is 2 pixles
    // 10x to magnify round-off errors
    // (thus target 10x radius is 20)
    unsigned long tenTimesDeltaLength = 
        intSqrt( (unsigned long)( 
                     10 * perpDelta.x * 10 * perpDelta.x + 
                     10 * perpDelta.y * 10 * perpDelta.y ) );
    
    intPair edgeAStart = add( centeredStart, perpDelta );
    intPair edgeAEnd = add( centeredEnd, perpDelta );
    
    intPair edgeBStart = subtract( centeredStart, perpDelta );
    intPair edgeBEnd = subtract( centeredEnd, perpDelta );
    
    if( tenTimesDeltaLength < 15 ) {
        // send one edge out a bit more, total diameter of three
        edgeAStart = add( edgeAStart, perpDelta );
        edgeAEnd = add( edgeAEnd, perpDelta );
        }
    else if( tenTimesDeltaLength > 25 ) {
        // bring one edge in a bit more

        // bring it in by half a delta
        perpDelta.x /= 2;
        perpDelta.y /= 2;

        edgeAStart = subtract( edgeAStart, perpDelta );
        edgeAEnd = subtract( edgeAEnd, perpDelta );
        }
    

    drawLine( bodyRGBA, w, edgeAStart, edgeAEnd, black );
    drawLine( bodyRGBA, w, edgeBStart, edgeBEnd, black );
    
    drawLine( bodyRGBA, w, edgeAStart, edgeBStart, black );
    drawLine( bodyRGBA, w, edgeAEnd, edgeBEnd, black );


    // now fill with white
    intPair fillStart = {8,8};
    
    floodFill( bodyRGBA, w, h, fillStart, white );    


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
        
        
        int *spriteX = new int[drawLimit];
        int *spriteY = new int[drawLimit];
        
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

            spriteX[i] = step.x - w/2;
            spriteY[i] = step.y - h/2;
            }
            

        drawSprite( p.mBodySpriteID, drawLimit, spriteX, spriteY, inColor );

        delete [] spriteX;
        delete [] spriteY;
        }
    else {
        printOut( "Arrow requested to draw not found\n" );
        }
    
    }


    
