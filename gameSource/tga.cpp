#include "tga.h"

#include <stdio.h>


rgbaColor *extractTGAData( unsigned char *inData, int inNumBytes,
                           int *outWidth, int *outHeight ) {

    int identificationFieldSize = inData[0];
    int offset = 18 + identificationFieldSize;

    if( inNumBytes < offset ) {
        printOut( "TGA data too small to even contain header.\n" );
        return NULL;
        }
    
    // read the color map type
    // (only 0, or no color map, is supported)
    if( inData[1] != 0 ) {
        printOut( "Only TGA files without colormaps can be read.\n" );
        return NULL;
        }
    
    // read the image type code
    // (only type 2, unmapped RGB image, is supported)
    if( inData[2] != 2 ) {
        printOut(
            "Only TGA files containing unmapped RGB images can be read.\n" );
        return NULL;
        }
    
    
    // ignore color map spec
    // (skip all 5 bytes of it)


    // now for the image specification

    // don't need either of these
    // x origin coordinate (2 bytes starting at inData[8])
    // y origin coordinate (2 bytes starting at inData[10])

    // little endian shorts
    int width = inData[13] << 8 | inData[12];
    int height = inData[15] << 8 | inData[14];

    *outWidth = width;
    *outHeight = height;
    
    int numPixels = width * height;

    
    // number of bits in pixels
    // only 24 bits per pixel supported
    
    if( inData[16] != 24 && inData[16] != 32 ) {
        printOut( "Only 24- and 32-bit TGA files can be read.\n" );
        return NULL;
        }

    int numChannels = 0;
    if( inData[16] == 24 ) {
        numChannels = 3;
        }
    else {
        numChannels = 4;
        }
    
    
    // image descriptor byte
    // setting to 0 specifies:
    // -- no attributes per pixel (for 24-bit)
    // -- screen origin in lower left corner
    // -- non-interleaved data storage
    // set bit 5 to 1 to specify screen origin in upper left corner
    char originAtTop = (char)( inData[17] & ( 1 << 5 ) );

    
    
    // We skip the image identification field
    // by using offset computed above

    
    // We also skip the color map data,
    // since we have none (as specified above).


    // make sure there's enough room for specified pixels
    int numRasterBytes = numPixels * numChannels;
    
    if( offset + numRasterBytes > inNumBytes ) {
        printOut( "TGA data too small to contain raster.\n" );
        return NULL;
        }
    
    
    // now we read the pixels, in BGR(A) order
    unsigned char *raster = &( inData[ offset ] );
    
    // return data
    rgbaColor *rgbaData = new rgbaColor[ numPixels ];
    
    
    int rasterIndex = 0;
    
    if( numChannels == 3 ) {
        if( originAtTop ) {
            for( int i=0; i<numPixels; i++ ) {
                rgbaData[i].b = raster[ rasterIndex++ ];
                rgbaData[i].g = raster[ rasterIndex++ ];
                rgbaData[i].r = raster[ rasterIndex++ ];
                rgbaData[i].a = 255;
                }
            }
        else {
            // we need to flip the raster vertically as we
            // copy it into our return image
            for( int y=height-1; y>=0; y-- ) {
                for( int x=0; x<width; x++ ) {
                    int imageIndex = y * width + x;
                    
                    rgbaData[ imageIndex ].b = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].g = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].r = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].a = 255;
                    }
                }
            }
        }
    else {  // numChannels == 4
                
        if( originAtTop ) {
            for( int i=0; i<numPixels; i++ ) {
                rgbaData[i].b = raster[ rasterIndex++ ];
                rgbaData[i].g = raster[ rasterIndex++ ];
                rgbaData[i].r = raster[ rasterIndex++ ];
                rgbaData[i].a = raster[ rasterIndex++ ];
                }
            }
        else {
            // we need to flip the raster vertically as we
            // copy it into our return image
            for( int y=height-1; y>=0; y-- ) {
                int yOffset = y * width;
                
                for( int x=0; x<width; x++ ) {
                    int imageIndex = yOffset + x;
                    
                    rgbaData[ imageIndex ].b = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].g = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].r = raster[ rasterIndex++ ];
                    rgbaData[ imageIndex ].a = raster[ rasterIndex++ ];
                    }
                }
            }
        }

    return rgbaData;
    }



rgbaColor *readTGAFile( char *inFileName, int *outWidth, int *outHeight ) {
    int fileDataSize;
    unsigned char *spriteFileData = readFile( inFileName, 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                outWidth, outHeight );

        delete [] spriteFileData;
        
        return spriteRGBA;
        }


    return NULL;
    }

    
