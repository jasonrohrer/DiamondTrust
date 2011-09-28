#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
#endif


#define true 1


#include <string.h>

#include "platform.h"
#include "common.h"
#include "measureChannel.h"
#include "cloneBoot.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"

// implement plaform functions


void* operator new ( std::size_t inSizeInBytes ) {
    if( inSizeInBytes == 0 ) {
        // return a 1-byte region here, since 0-allocs crash on the DS
        // this ensures that the delete operator works without case-checking
        // (instead of returning NULL)
        inSizeInBytes = 1;
        }

    return OS_Alloc( inSizeInBytes );
    }


void* operator new[] ( std::size_t inSizeInBytes ) {
    if( inSizeInBytes == 0 ) {
        // return a 1-byte region here, since 0-allocs crash on the DS
        // this ensures that the delete operator works without case-checking
        // (instead of returning NULL)
        inSizeInBytes = 1;
        }
    
    return OS_Alloc( inSizeInBytes );
    }


void operator delete ( void *inRegion ) throw() {
    OS_Free( inRegion );
    }


void operator delete[] ( void *inRegion ) throw() {
    OS_Free( inRegion );
    }


// called by system before C++ static constructor calls
// set our heap system up here for use by new and delete operators
void NitroStartUp( void ) {
    OS_Init();
    

    // setup memory heap    
    OS_Printf( "Setting up heap\n" );
    void *arenaLo = OS_InitAlloc( OS_ARENA_MAIN, 
                                  OS_GetMainArenaLo(),
                                  OS_GetMainArenaHi(), 1 );

    OS_SetArenaLo( OS_ARENA_MAIN, arenaLo );

    OSHeapHandle heapHandle = OS_CreateHeap( OS_ARENA_MAIN, 
                                             OS_GetMainArenaLo(), 
                                             OS_GetMainArenaHi() );
    if( heapHandle < 0 ) {
        OS_TPanic( "Failed to create main heap.\n");
        }

    OS_SetCurrentHeap( OS_ARENA_MAIN, heapHandle );
    }


// tries to start next pending network send
void stepNetwork();




// true if we are a clone-booted child with no local file system
char isCloneChild = false;


#include "../dataFiles.cpp"

unsigned char *readFile( const char *inFileName, int *outSize ) {

    // check if data for this file is inline-included
    unsigned char *inlinedData = readIncludedFile( inFileName, outSize );
    
    if( inlinedData != NULL ) {
        return inlinedData;
        }
        
    // else try other methods of reading file data



    if( isCloneChild ) {
        // no local file system, send requests for files to parent

        printOut( "Child requesting file '%s' from parent...\n", inFileName );

        // channel 1 for file requests
        sendMessage( (unsigned char *)inFileName, strlen( inFileName ), 1 );
    
        unsigned char *message = NULL;

        unsigned int length;
        
        // poll for response message
        while( message == NULL ) {
            stepNetwork();
            message = getMessage( &length, 1 );
            }
        
        if( length != 4 ) {
            printOut( " ...child received bad file header\n" );
            delete [] message;
            return NULL;
            }
        
        unsigned int numChunks =
            (unsigned int)( message[0] << 24 ) |
            (unsigned int)( message[1] << 16 ) |
            (unsigned int)( message[2] << 8 ) |
            (unsigned int)( message[3] );
        
        delete [] message;
        
        printOut( " ...child recieving file in %d chunks\n", numChunks );
        
        //printOut( "Allocating vector\n" );
        
        SimpleVector<unsigned char> fileContents;
        
        for( int i=0; i<numChunks; i++ ) {
            // wait for next chunk to arrive
            message = NULL;
            
            while( message == NULL ) {
                stepNetwork();
                message = getMessage( &length, 1 );
                }
            
            fileContents.push_back( message, (int)length );
            delete [] message;
            }
        
        unsigned char *returnData = fileContents.getElementArray();
        


        printOut( "  ...child got complete file \n" );


        *outSize = fileContents.size();
        return returnData;
        }



    FSFile file;
    FS_InitFile( &file );
    
    if( FS_OpenFileEx( &file, inFileName, FS_FILEMODE_R ) ) {
        unsigned int length = FS_GetLength( &file );
        
        unsigned char *buffer = new unsigned char[ length ];
        
        int numRead = FS_ReadFile( &file, buffer, (int)length );
        
        if( numRead != length ) {
            delete [] buffer;
            buffer = NULL;
            *outSize = -1;
            }
        else {
            *outSize = numRead;
            }
        
        FS_CloseFile( &file );

        return buffer;
        }
    else {
        *outSize = -1;
        return NULL;
        }
    
    }




char isDirectory( const char *inFileName ) {
    FSFile file;
    FS_InitFile( &file );

    char returnVal = false;
    
    if( FS_FindDir( &file, inFileName ) ) {
        returnVal = true;
        
        FS_CloseDirectory( &file );
        }
    
    return returnVal;
    }



char **listDirectory( const char *inFileName, int *outNumEntries ) {
    FSFile file;
    FS_InitFile( &file );
    
    char returnVal = false;
    
    if( FS_FindDir( &file, inFileName ) ) {
            
        FSDirEntry dirEntry;
        
        SimpleVector<char *> dirFileNames;
        
        while( FS_ReadDir( &file, &dirEntry ) ) {
            
            // completely ignore "hidden" files
            // this avoids trouble caused by .DS_Store files created on Macs
            if( strlen( dirEntry.name ) > 0 && dirEntry.name[0] != '.' ) {    
                char *name = autoSprintf( "%s/%s", inFileName, dirEntry.name );
            
                dirFileNames.push_back( name );
                }
            
            }

        FS_CloseDirectory( &file );

        *outNumEntries = dirFileNames.size();
        return dirFileNames.getElementArray();
        
        }
    
    return NULL;
    }

    

FileHandle openFile( const char *inFileName, int *outSize ) {

    FSFile *file = new FSFile;
    FS_InitFile( file );
    
    if( FS_OpenFileEx( file, inFileName, FS_FILEMODE_R ) ) {
        
        *outSize = (int)FS_GetLength( file );

        return file;
        }
    

    delete file;

    return NULL;
    }



int readFile( FileHandle inFile, unsigned char *inBuffer, int inBytesToRead ) {
    FSFile *file = (FSFile *)inFile;

    // split large read up to allow vblank interrupt and other stuff to run

    int maxReadSize = 512;
    
    if( inBytesToRead <= maxReadSize ) {
        return FS_ReadFile( file, inBuffer, inBytesToRead );
        }
    else {
        int totalRead = 0;

        while( inBytesToRead > 0 ) {
            int numToRead = inBytesToRead;
            
            if( numToRead > maxReadSize ) {
                numToRead = maxReadSize;
                }
            
            totalRead += FS_ReadFile( file, inBuffer, numToRead );
            
            inBuffer = &( inBuffer[ numToRead ] );
            inBytesToRead -= numToRead;
            }
        return totalRead;
        }
    
    }



void closeFile( FileHandle inFile ) {
    FSFile *file = (FSFile *)inFile;
    
    FS_CloseFile( file );
    
    delete file;
    }



void fileSeek( FileHandle inFile, int inAbsolutePosition ) {
    FSFile *file = (FSFile *)inFile;

    BOOL result = FS_SeekFile( file, inAbsolutePosition, FS_SEEK_SET );
    
    if( ! result ) {
        printOut( "FS_SeekFile failed\n" );
        }
    }









void printOut( const char *inFormatString, ... ) {
    va_list argList;
    va_start( argList, inFormatString );

    
    //OS_VPrintf( inFormatString, argList );
    // simpler to reduce stack overhead
    OS_TVPrintf( inFormatString, argList );
    
    va_end( argList );
    }




unsigned int getSecondsSinceEpoc() {
    RTCDate date;
    RTCTime time;
    
    RTC_GetDate( &date );
    RTC_GetTime( &time );
    
    s64 seconds = RTC_ConvertDateTimeToSecond( &date , &time );
    
    return (unsigned int)seconds;
    }


unsigned int getSystemMilliseconds() {
    return (unsigned int)( OS_TicksToMilliSeconds( OS_GetTick() ) );
    }




//#define MAX_TEXTURES  512
#define MAX_TEXTURES  1024

struct textureInfoStruct {
        int spriteHandle;

        unsigned int slotAddress;
        GXTexFmt texFormat;
        // if format is DIRECT, this is not set:
        unsigned int paletteSlotAddress;
        char colorZeroTransparent;
        int w;
        int h;
        int numTextureDataBytes;
        // number of bytes in w pixels
        int numTextureBytesPerLine;
        
  
        // -1 if not in a set
        int textureSetID;
        char activeInSet;
        char dataInTextureRAM;
        
        // this is NULL if not in a set
        unsigned char *textureDataCopy;
        
        GXTexSizeS sizeS;
        GXTexSizeT sizeT;
        GXSt texCoordCorners[4];
    };

typedef struct textureInfoStruct textureInfo;

textureInfo textureInfoArray[ MAX_TEXTURES ];

int nextTextureInfoIndex = 0;
unsigned int nextTextureSlotAddress = 0x0000;
unsigned int nextTexturePaletteAddress = 0x0000;

unsigned int numTextureBytesAdded = 0;
unsigned int numTexturePaletteBytesAdded = 0;


typedef struct textureSet {
        SimpleVector<int> *setMembers;
        int activeMember;
    } textureSet;

SimpleVector<textureSet> textureSetVector;




// resets all texture information back to starting state, as if
// no sprites have been added at all
static void clearAllTextures() {
    nextTextureInfoIndex = 0;
    nextTextureSlotAddress = 0x0000;
    nextTexturePaletteAddress = 0x0000;
    
    numTextureBytesAdded = 0;
    numTexturePaletteBytesAdded = 0;
    
    textureSetVector.deleteAll();
    }




// used as temporary storage when adding a sprite to build up a palette
unsigned short textureColors[ 256 ] ATTRIBUTE_ALIGN(4);




static textureInfo makeTextureInfo( int inWidth, int inHeight, 
                                    int inSetID = -1 ) {
    textureInfo t;
    t.textureSetID = inSetID;
    

    // existing base to use from set?
    textureInfo *baseT = NULL;
    
    if( t.textureSetID != -1 ) {
        textureSet *s = textureSetVector.getElement( t.textureSetID );
    
        if( s->activeMember != -1 ) {
            // members of set already exist
            baseT = &textureInfoArray[ s->activeMember ];
            }
        }

    if( baseT != NULL ) {
        // reuse slot address
        t.slotAddress = baseT->slotAddress;
        }
    else {
        // new slot address
        t.slotAddress = nextTextureSlotAddress;
        }
    

    t.w = inWidth;
    t.h = inHeight;
    
    //printOut( "Adding %dx%d texture at address %d, handle %d\n", 
    //          t.w, t.h, t.slotAddress, nextTextureInfoIndex );
    

    switch( inWidth ) {
        case 8:
            t.sizeS = GX_TEXSIZE_S8;
            break;
        case 16:
            t.sizeS = GX_TEXSIZE_S16;
            break;
        case 32:
            t.sizeS = GX_TEXSIZE_S32;
            break;
        case 64:
            t.sizeS = GX_TEXSIZE_S64;
            break;
        case 128:
            t.sizeS = GX_TEXSIZE_S128;
            break;
        case 256:
            t.sizeS = GX_TEXSIZE_S256;
            break;
        case 512:
            t.sizeS = GX_TEXSIZE_S512;
            break;
        case 1024:
            t.sizeS = GX_TEXSIZE_S1024;
            break;
        default:
            printOut( "Unsupported texture width, %d\n", inWidth );
        }
    switch( inHeight ) {
        case 8:
            t.sizeT = GX_TEXSIZE_T8;
            break;
        case 16:
            t.sizeT = GX_TEXSIZE_T16;
            break;
        case 32:
            t.sizeT = GX_TEXSIZE_T32;
            break;
        case 64:
            t.sizeT = GX_TEXSIZE_T64;
            break;
        case 128:
            t.sizeT = GX_TEXSIZE_T128;
            break;
        case 256:
            t.sizeT = GX_TEXSIZE_T256;
            break;
        case 512:
            t.sizeT = GX_TEXSIZE_T512;
            break;
        case 1024:
            t.sizeT = GX_TEXSIZE_T1024;
            break;
        default:
            printOut( "Unsupported texture height, %d\n", inHeight );
        }
    
    t.texCoordCorners[0] = GX_ST( 0, inHeight << FX32_SHIFT );
    t.texCoordCorners[1] = GX_ST( 0, 0 );
    t.texCoordCorners[2] = GX_ST( inWidth << FX32_SHIFT,  0 );
    t.texCoordCorners[3] = GX_ST( inWidth << FX32_SHIFT,  
                                  inHeight << FX32_SHIFT );

    t.numTextureDataBytes = 0;
    t.activeInSet = false;
    t.dataInTextureRAM = false;
    t.textureDataCopy = NULL;

    return t;
    }




static void addPalette( textureInfo *inT, 
                        unsigned short *inPalette, 
                        unsigned int inPaletteEntries ) {
    
    inT->paletteSlotAddress = nextTexturePaletteAddress;

    // copy into aligned memory
    unsigned int paletteBytes = inPaletteEntries * sizeof( short );
    
    memcpy( (void *)textureColors, (void *)inPalette, paletteBytes );

    

    DC_FlushRange( textureColors, paletteBytes );

    int vCount = GX_GetVCount();
        
    // palettes are always small enough to load entirely in just a few
    // vlines 256 entries max, 16 bits each... 512 bytes total
    // 213 is the last safe vline, so give a buffer of 4 vlines to be safe
    // (cover DMA overhead)
    if( vCount < 191 || vCount > 209 ) {
        // not in safe blanking region
        OS_WaitVBlankIntr();
        }
    vCount = GX_GetVCount();
    
    
    GX_BeginLoadTexPltt();
    GX_LoadTexPltt( textureColors,
                    inT->paletteSlotAddress,
                    paletteBytes );
    GX_EndLoadTexPltt();
    
    int endVCount = GX_GetVCount();

    if( endVCount > 213 || endVCount < 191 ) {
        printOut( "*** WARNING: texture palette DMA ended "
                  "outside of VBlank\n" );
        }
    

    unsigned int offset = paletteBytes;
    
    if( offset < 16 ) {
        // offset to next palette must be 16 bytes for 
        //   16- and 256-color palettes
        // so our 8-byte offset (for this 4-color palette) is
        // not enough.
        offset = 16;
        }
    
    nextTexturePaletteAddress += offset;
    
    numTexturePaletteBytesAdded += offset;
    printOut( "Added %d paletteBytes bytes so far.\n", 
              numTexturePaletteBytesAdded );
    }




static void replaceTextureData( textureInfo *inT,
                                unsigned short *inPackedTextureData,
                                unsigned int inNumBytes,
                                unsigned int inByteOffset = 0 ) {

    unsigned char *dataPointer = (unsigned char*)inPackedTextureData;

    // skip
    dataPointer = &( dataPointer[ inByteOffset ] );
    

    DC_FlushRange( dataPointer, inNumBytes );


    unsigned int numBytesLeft = inNumBytes;
    unsigned int numDone = 0;
    
    while( numBytesLeft > 0 ) {

        int vCount = GX_GetVCount();
        
        if( vCount < 191 || vCount > 213 ) {
            // not in safe blanking region
            OS_WaitVBlankIntr();
            }
        vCount = GX_GetVCount();
        
        int vLinesLeft = 213 - vCount;
        
        // account for some overhead in transfer to be safe
        vLinesLeft -= 2;
        
        // measurements showed that something like this was safe
        // theoretical limit is 2840 bytes per vline (assuming 32-bit DMA mode)
        unsigned int bytesPerVLine = 2000;

        if( vLinesLeft > 0 ) {
            


            unsigned int maxTransferSize = bytesPerVLine * vLinesLeft;
        
            unsigned int numToTransfer = numBytesLeft;
        
            if( numToTransfer > maxTransferSize ) {
                numToTransfer = maxTransferSize;
                }
        

            GX_BeginLoadTex();
    
            GX_LoadTex( &( dataPointer[ numDone ] ), 
                        inT->slotAddress + inByteOffset + numDone,
                        numToTransfer );
            
            GX_EndLoadTex();

            int endVCount = GX_GetVCount();

            if( endVCount > 213 || endVCount < 191 ) {
                printOut( "*** WARNING: texture data DMA ended "
                          "outside of VBlank\n" );
                }

            
            numDone += numToTransfer;
            numBytesLeft -= numToTransfer;
            }
        }
    }



static void addTextureData( textureInfo *inT,
                            unsigned short *inPackedTextureData,
                            unsigned int inNumBytes ) {
    
    replaceTextureData( inT, inPackedTextureData, inNumBytes );
    inT->dataInTextureRAM = true;

    
    inT->numTextureDataBytes = (int)inNumBytes;

    if( inT->textureSetID != -1 ) {
        // save the texture data
        inT->textureDataCopy = new unsigned char[ inNumBytes ];
        memcpy( inT->textureDataCopy, 
                (void *)inPackedTextureData, inNumBytes );
        }


     // existing base to use from set?
    textureInfo *baseT = NULL;
    
    if( inT->textureSetID != -1 ) {
        textureSet *s = textureSetVector.getElement( inT->textureSetID );
        
        if( s->activeMember != -1 ) {
            // members of set already exist
            baseT = &textureInfoArray[ s->activeMember ];
            }
        }
    
    
    if( baseT == NULL ) {
        // new addtion

        nextTextureSlotAddress += inNumBytes;
    
        numTextureBytesAdded += inNumBytes;
        }
    
    }



static void addToSet( textureInfo *t ) {
    if( t->textureSetID != -1 ) {
        printOut( "Adding sprite to set %d\n", t->textureSetID );
        
        // add to set
        textureSet *s = textureSetVector.getElement( t->textureSetID );

        s->setMembers->push_back( t->spriteHandle );
        

        // make latest active in set
        if( s->activeMember != -1 ) {
            textureInfoArray[ s->activeMember ].activeInSet = false;
            }
        
        s->activeMember = t->spriteHandle;
        
        t->activeInSet = true;
        }
    }



int addSprite256( unsigned char *inDataBytes, int inWidth, int inHeight,
                  unsigned short inPalette[256], char inZeroTransparent, 
                  int inSetID ) {
    
    unsigned int numPixels = (unsigned int)( inWidth * inHeight );
    
    textureInfo t = makeTextureInfo( inWidth, inHeight, inSetID );

    t.texFormat = GX_TEXFMT_PLTT256;
    t.numTextureBytesPerLine = inWidth;
    
    addPalette( &t, inPalette, 256 );
    
    addTextureData( &t, (unsigned short*)inDataBytes, numPixels );
    
    t.colorZeroTransparent = inZeroTransparent;

    t.spriteHandle = nextTextureInfoIndex;
    textureInfoArray[ nextTextureInfoIndex ] = t;
    int returnIndex = nextTextureInfoIndex;
    
    nextTextureInfoIndex++;

    addToSet( & textureInfoArray[ returnIndex ] );
    
    return returnIndex;
    }


typedef struct pendingReplacement {
        int spriteID;
        unsigned char *dataBytes;
        char deleteDataWhenDone;
        unsigned int numBytes;
        unsigned int numBytesDone;
        unsigned int blockSize;
    } pendingReplacement;

SimpleVector<pendingReplacement> textureReplacements;

static void textureReplacementStep() {
    if( textureReplacements.size() > 0 ) {
        pendingReplacement *p = textureReplacements.getElement( 0 );
        
        // send one block
        replaceTextureData( &( textureInfoArray[ p->spriteID ] ), 
                            (unsigned short*)p->dataBytes, p->blockSize,
                            p->numBytesDone );
        
        p->numBytesDone += p->blockSize;
        
        if( p->numBytesDone == p->numBytes ) {
            // finished
            textureInfoArray[ p->spriteID ].dataInTextureRAM = true;
            
            if( p->deleteDataWhenDone ) {
                delete [] p->dataBytes;
                }
            
            textureReplacements.deleteElement( 0 );
            }        
        }
    }

    

void replaceSprite256( int inSpriteID, 
                       unsigned char *inDataBytes, 
                       int inWidth, int inHeight, char inReplaceSafe ) {
    
    makeSpriteActive( inSpriteID );

    unsigned int numPixels = (unsigned int)( inWidth * inHeight );

    if( inReplaceSafe ) {
        
        // copy the data
        unsigned char *data = new unsigned char[ numPixels ];
        memcpy( data, inDataBytes, numPixels );
        
        pendingReplacement p = 
            { inSpriteID, data, true, numPixels, 0, (unsigned int)inWidth };
        textureReplacements.push_back( p );
        }
    else {    
        replaceTextureData( &( textureInfoArray[ inSpriteID ] ), 
                            (unsigned short*)inDataBytes, numPixels );

        textureInfoArray[ inSpriteID ].dataInTextureRAM = true;
        }
    
    }



int createSpriteSet() {
    textureSet s;
    s.setMembers = new SimpleVector<int>();
    s.activeMember = -1;
    textureSetVector.push_back( s );

    return textureSetVector.size() - 1;
    }



void makeSpriteActive( int inSpriteID, char inReplaceSafe ) {
    textureInfo *t = &( textureInfoArray[ inSpriteID ] );

    int setID = t->textureSetID;
    

    if( setID != -1 && ! t->activeInSet ) {

        if( !inReplaceSafe ) {
            
            // load into texture RAM right now
            replaceTextureData( t, (unsigned short *)( t->textureDataCopy ), 
                                (unsigned int)t->numTextureDataBytes );
            t->dataInTextureRAM = true;
            }
        else {
            t->dataInTextureRAM = false;

            // put in queue for safe copy into texture RAM
            
            // 8 lines at a time (all images have H that is multiple of 8)
            unsigned int blockSize = 
                (unsigned int)( t->numTextureBytesPerLine * 8 );
            if( t->h >8 ) {
                // multiples of 16 lines
                blockSize *= 2;
                }
            if( t->h > 16 ) {
                // multiples of 32 lines
                blockSize *=2;
                }
            if( t->h > 32 ) {
                // multiples of 64 lines
                blockSize *= 2;
                }
            
            // cap at 2048 bytes to avoid potential screen artifacts
            while( blockSize > 2048 ) {
                blockSize /= 2;
                }
            

            pendingReplacement p = 
            { inSpriteID, t->textureDataCopy, false, 
              (unsigned int)t->numTextureDataBytes, 0, blockSize };
            textureReplacements.push_back( p );
            }
        
        t->activeInSet = true;
        
        
        // mark last active no longer active
        textureSet *s = textureSetVector.getElement( setID );
        
        if( s->activeMember != -1 ) {
            textureInfoArray[ s->activeMember ].activeInSet = false;
            textureInfoArray[ s->activeMember ].dataInTextureRAM = false;
            }
        
        s->activeMember = inSpriteID;
        }
    }


char isSpriteReady( int inHandle ) {
    return textureInfoArray[ inHandle ].dataInTextureRAM;
    }

    
    



int addSprite( rgbaColor *inDataRGBA, int inWidth, int inHeight,
               int inSetID ) {
    
    textureInfo t = makeTextureInfo( inWidth, inHeight, inSetID );
        



    unsigned int numPixels = (unsigned int)( inWidth * inHeight );
    
    unsigned short *textureData = new unsigned short[ numPixels ];

    // track palette indices for each pixel, even if we don't end up
    // using it
    unsigned char *texturePaletteIndices =  new unsigned char[ numPixels ];
    

    // zero palette (so unused colors are 0 later)
    for( int k=0; k<256; k++ ) {
        textureColors[k] = 0;
        }

    // reserve color 0 for transparency, but only if it's used!
    // check
    char transUsed = false;
    
    for( int i=0; i<numPixels && !transUsed; i++ ) {
        if( inDataRGBA[i].a == 0 ) {
            transUsed = true;
            }
        }
    
    int numUniqueColors = 0;
    
    if( transUsed ) {
        // 0 already set asside for all transparent pixels
        numUniqueColors = 1;
        }
    

    for( int i=0; i<numPixels; i++ ) {
        // discard lowest 3 bits for each color
        // discard all but 1 bit for alpha
        
        rgbaColor c = inDataRGBA[i];
        
        textureData[i] = (unsigned short)( 
            ( c.a >> 7 ) << 15 |
            ( c.b >> 3 ) << 10 |
            ( c.g >> 3 ) << 5 |
            ( c.r >> 3 ) );
        
        if( numUniqueColors < 257 ) {
            
            // unique?
            unsigned short c16 = textureData[i];

            char found = false;

            if( transUsed ) {    
                int alpha = c.a >> 7;
                if( alpha == 0 ) {
                    // matches palette color 0
                    texturePaletteIndices[i] = 0;
                    found = true;
                    }
                }
            
            
            int jLimit = numUniqueColors;
            if( jLimit > 256 ) {
                jLimit = 256;
                }
            
            int startColorIndex = 0;
            if( transUsed ) {
                startColorIndex = 1;
                }
            
            for( int j=startColorIndex; j<jLimit && !found; j++ ) {
                if( c16 == textureColors[ j ] ) {
                    found = true;
                    texturePaletteIndices[i] = (unsigned char)j;
                    }
                }
            if( !found ) {
                
                if( numUniqueColors < 256 ) {
                    // add to palette
                    textureColors[ numUniqueColors ] = c16;                    
                    texturePaletteIndices[i] = (unsigned char)numUniqueColors;
                    //printOut( "Adding color to palette: " );
                    //printColor( c );
                    //printOut( "\n" );
                    }
                // this may push us past 256, in which case
                // we cannot use a palette for this texture
                numUniqueColors ++;
                }
            }
        
        }
    

    unsigned short *packedTextureData = NULL;
    unsigned int numTextureShorts;
    

    if( numUniqueColors > 256 ) {
        // direct color
        //printOut( "Texture requires direct color\n" );

        t.texFormat = GX_TEXFMT_DIRECT;
        t.numTextureBytesPerLine = 2 * inWidth;

        numTextureShorts = numPixels;
        packedTextureData = textureData;

        // only use color zero as transparent for paletted textures
        // (direct color textures have 1-bit alpha)
        t.colorZeroTransparent = false;        
        }
    else {
        // force zero transparent for paletted textures if trans used
        t.colorZeroTransparent = transUsed;
        
        //printOut( "Texture uses only %d colors\n", numUniqueColors );
        
        // palette colors are RGB 555 (alpha bit (bit 15) ignored... set to
        // 1 just to be safe
        for( int k=0; k<numUniqueColors; k++ ) {
            textureColors[k] |= 0x8000;
            }

        // don't need direct colors anymore
        delete [] textureData;

        unsigned int paletteSize = 0;
        
        if( numUniqueColors <= 4 ) {
            t.texFormat = GX_TEXFMT_PLTT4;
            t.numTextureBytesPerLine = inWidth / 4;

            paletteSize = 4;
            
            // pack into 2 bits per pixel (8 pixels per short)
            numTextureShorts = numPixels / 8;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<8; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 2)) );
                    }
                }
            }
        else if( numUniqueColors <= 16 ) {
            t.texFormat = GX_TEXFMT_PLTT16;
            t.numTextureBytesPerLine = inWidth / 2;

            paletteSize = 16;
            
            // pack into 4 bits per pixel (4 pixels per short)
            numTextureShorts = numPixels / 4;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<4; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 4)) );
                    }
                }
            }
        else if( numUniqueColors <= 256 ) {
            t.texFormat = GX_TEXFMT_PLTT256;
            t.numTextureBytesPerLine = inWidth;

            paletteSize = 256;
            
            // pack into 8 bits per pixel (2 pixel per short)
            numTextureShorts = numPixels / 2;
            packedTextureData = new unsigned short[ numTextureShorts ];
            
            int pixIndex = 0;
            
            for( int i=0; i<numTextureShorts; i++ ) {
                packedTextureData[i] = 0;
                
                for( int j=0; j<2; j++ ) {
                    unsigned int palIndex = 
                        texturePaletteIndices[ pixIndex++ ];
                    packedTextureData[i] = (unsigned short)(
                        packedTextureData[i] | (palIndex << (j * 8)) );
                    }
                }
            }
        
        //printOut( "Using a %d color palette\n", paletteSize );


        

        t.paletteSlotAddress = nextTexturePaletteAddress;

        addPalette( &t, textureColors, paletteSize );
        }
    

    unsigned int numTextureBytes = numTextureShorts * 2;
    addTextureData( &t, packedTextureData, numTextureBytes );



    
    t.spriteHandle = nextTextureInfoIndex;
    textureInfoArray[ nextTextureInfoIndex ] = t;
    int returnIndex = nextTextureInfoIndex;
    
    nextTextureInfoIndex++;
    

    addToSet( & textureInfoArray[ returnIndex ] );        

    
    printOut( "Added %d texture bytes so far (last texture had %d colors).\n",
              numTextureBytesAdded, numUniqueColors );
    
    delete [] packedTextureData;
    
    delete [] texturePaletteIndices;
    
    return returnIndex;
    }



// fake z buffer values to force proper sorting (later sprites layers on top)
fx16 farZ = -7 * FX16_ONE;
fx16 drawZ = farZ;
fx16 drawZIncrement = 0x0004;

int polyID = 0;


void startNewSpriteLayer() {
    drawZ += drawZIncrement;

    // also start a new ID group 
    polyID++;
    if( polyID > 63 ) {
        polyID = 0;
        }
    }



void drawSprite( int inHandle, int inX, int inY, rgbaColor inColor ) {
    //if( true ) return;

    makeSpriteActive( inHandle );
    

    textureInfo t = textureInfoArray[ inHandle ];
    
    //printOut( "Drawing sprite at address %d\n", t.slotAddress );


    GXTexPlttColor0 colorZeroFlag;
    if( ! t.colorZeroTransparent ) {
        colorZeroFlag = GX_TEXPLTTCOLOR0_USE;
        }
    else {
        colorZeroFlag = GX_TEXPLTTCOLOR0_TRNS;
        }
    
    G3_TexImageParam( t.texFormat,
                      GX_TEXGEN_TEXCOORD,
                      t.sizeS,
                      t.sizeT,
                      GX_TEXREPEAT_NONE,
                      GX_TEXFLIP_NONE,
                      colorZeroFlag,
                      t.slotAddress );
    
    
    if( t.texFormat != GX_TEXFMT_DIRECT ) {        
        G3_TexPlttBase( t.paletteSlotAddress,
                        t.texFormat );
        }
    
    
    // 5 high-order bits
    int a = inColor.a >> 3;
    
    
    // avoid wireframe
    if( a == 0 ) {
        // draw nothing
        return;
        }


    G3_PolygonAttr( GX_LIGHTMASK_NONE,
                    GX_POLYGONMODE_MODULATE,
                    GX_CULL_NONE,
                    polyID,
                    a,
                    0 );

    G3_PushMtx();


    G3_Translate( 0, 
                  0, 
                  drawZ );



    G3_Begin( GX_BEGIN_QUADS );

    // set color once
    G3_Color( GX_RGB( inColor.r >> 3, inColor.g >> 3, inColor.b >> 3 ) );


    // four corners

    // 16 bit fixed point not enough to hold integer values in range 0..255
    // (screen pixel coordinates) so some shifting is necessary
    // Make up for this in the setup of Ortho in main function
    // (shifting by 6 is like dividing by 64, and 256x192 divided by 64
    //   is 4x3, which is used in Ortho).

    // Note that this was the ONLY coordinate conversion method that didn't
    // result in distortion as polygons moved in y direction.

    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[0] );
    G3_Vtx( (short)( inX << (FX16_SHIFT - 6) ), 
            (short)( (inY + t.h) << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[1] );
    G3_Vtx( (short)( inX << (FX16_SHIFT - 6) ), 
            (short)( inY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[2] );
    G3_Vtx( (short)( (inX + t.w) << (FX16_SHIFT - 6) ), 
            (short)( inY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[3] );
    G3_Vtx( (short)( (inX + t.w) << (FX16_SHIFT - 6) ), 
            (short)( (inY + t.h) << (FX16_SHIFT - 6) ), 
            0 );

    G3_End();

    G3_PopMtx( 1 );
    }





void drawSprite( int inHandle, int inNumCopies, int inX[], int inY[], 
                 rgbaColor inColor ) {
    //if( true ) return;

    makeSpriteActive( inHandle );
    

    textureInfo t = textureInfoArray[ inHandle ];
    
    //printOut( "Drawing sprite at address %d\n", t.slotAddress );


    GXTexPlttColor0 colorZeroFlag;
    if( ! t.colorZeroTransparent ) {
        colorZeroFlag = GX_TEXPLTTCOLOR0_USE;
        }
    else {
        colorZeroFlag = GX_TEXPLTTCOLOR0_TRNS;
        }
    
    G3_TexImageParam( t.texFormat,
                      GX_TEXGEN_TEXCOORD,
                      t.sizeS,
                      t.sizeT,
                      GX_TEXREPEAT_NONE,
                      GX_TEXFLIP_NONE,
                      colorZeroFlag,
                      t.slotAddress );
    
    
    if( t.texFormat != GX_TEXFMT_DIRECT ) {        
        G3_TexPlttBase( t.paletteSlotAddress,
                        t.texFormat );
        }
    
    
    // 5 high-order bits
    int a = inColor.a >> 3;
    
    
    // avoid wireframe
    if( a == 0 ) {
        // draw nothing
        return;
        }


    G3_PolygonAttr( GX_LIGHTMASK_NONE,
                    GX_POLYGONMODE_MODULATE,
                    GX_CULL_NONE,
                    polyID,
                    a,
                    0 );

    G3_PushMtx();


    G3_Translate( 0, 
                  0, 
                  drawZ );



    G3_Begin( GX_BEGIN_QUADS );

    // set color once
    G3_Color( GX_RGB( inColor.r >> 3, inColor.g >> 3, inColor.b >> 3 ) );


    // four corners

    // 16 bit fixed point not enough to hold integer values in range 0..255
    // (screen pixel coordinates) so some shifting is necessary
    // Make up for this in the setup of Ortho in main function
    // (shifting by 6 is like dividing by 64, and 256x192 divided by 64
    //   is 4x3, which is used in Ortho).

    // Note that this was the ONLY coordinate conversion method that didn't
    // result in distortion as polygons moved in y direction.

    for( int i=0; i<inNumCopies; i++ ) {
        int x = inX[i];
        int y = inY[i];
        
        G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[0] );
        G3_Vtx( (short)( x << (FX16_SHIFT - 6) ), 
                (short)( (y + t.h) << (FX16_SHIFT - 6) ), 
                0 );
    
        G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[1] );
        G3_Vtx( (short)( x << (FX16_SHIFT - 6) ), 
                (short)( y << (FX16_SHIFT - 6) ), 
                0 );
    
        G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[2] );
        G3_Vtx( (short)( (x + t.w) << (FX16_SHIFT - 6) ), 
                (short)( y << (FX16_SHIFT - 6) ), 
                0 );
    
        G3_Direct1( G3OP_TEXCOORD, t.texCoordCorners[3] );
        G3_Vtx( (short)( (x + t.w) << (FX16_SHIFT - 6) ), 
                (short)( (y + t.h) << (FX16_SHIFT - 6) ), 
                0 );
        }

    G3_End();

    G3_PopMtx( 1 );
    }



void drawRect( int inStartX, int inStartY, int inEndX, int inEndY, 
               rgbaColor inColor ) {
    
    // disable texture mapping
    G3_TexImageParam( GX_TEXFMT_NONE,
                      GX_TEXGEN_NONE,
                      GX_TEXSIZE_S8,
                      GX_TEXSIZE_T8,
                      GX_TEXREPEAT_NONE, 
                      GX_TEXFLIP_NONE, 
                      GX_TEXPLTTCOLOR0_USE, 
                      0 );

    // 5 high-order bits
    int a = inColor.a >> 3;
    
    
    // avoid wireframe
    if( a == 0 ) {
        // draw nothing
        return;
        }


    G3_PolygonAttr( GX_LIGHTMASK_NONE,
                    GX_POLYGONMODE_MODULATE,
                    GX_CULL_NONE,
                    polyID,
                    a,
                    0 );

    G3_PushMtx();


    G3_Translate( 0, 
                  0, 
                  drawZ );



    G3_Begin( GX_BEGIN_QUADS );

    // set color once
    G3_Color( GX_RGB( inColor.r >> 3, inColor.g >> 3, inColor.b >> 3 ) );
    

    // four corners

    // 16 bit fixed point not enough to hold integer values in range 0..255
    // (screen pixel coordinates) so some shifting is necessary
    // Make up for this in the setup of Ortho in main function
    // (shifting by 6 is like dividing by 64, and 256x192 divided by 64
    //   is 4x3, which is used in Ortho).

    // Note that this was the ONLY coordinate conversion method that didn't
    // result in distortion as polygons moved in y direction.

    G3_Vtx( (short)( inStartX << (FX16_SHIFT - 6) ), 
            (short)( inEndY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Vtx( (short)( inStartX << (FX16_SHIFT - 6) ), 
            (short)( inStartY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Vtx( (short)( inEndX << (FX16_SHIFT - 6) ), 
            (short)( inStartY << (FX16_SHIFT - 6) ), 
            0 );
    
    G3_Vtx( (short)( inEndX << (FX16_SHIFT - 6) ), 
            (short)( inEndY << (FX16_SHIFT - 6) ), 
            0 );

    G3_End();

    G3_PopMtx( 1 );
    }




#define TP_SAMPLE_BUFFER_SIZE 5

static TPData tpAutoSampleBuffer[ TP_SAMPLE_BUFFER_SIZE ];


// don't miss any touches, even after they're over, until they are consumed
// by getTouch
char touchWasDown = false;

int touchX = 0;
int touchY = 0;

static void autoTouchCallback( TPRequestCommand inCommand, 
                               TPRequestResult inResult,
                               u16 inIndex ) {
    
    if( inCommand == TP_REQUEST_COMMAND_AUTO_SAMPLING &&
        inResult == TP_RESULT_SUCCESS ) {
        
        TPData data;
        
        TP_GetLatestCalibratedPointInAuto( &data );
    
        // clear up unused variable warning
        inIndex = 0;
    
        if( data.touch == TP_TOUCH_ON && 
            data.validity == TP_VALIDITY_VALID ) { 
        
            touchWasDown = true;
            
            touchX = data.x;
            touchY = data.y;
            }
        
        
        }
    }





char getTouch( int *outX, int *outY ) {
    if( getLidClosed() ) {
        // block/discard all input when lid is closed
        touchWasDown = false;
        
        return false;
        }
    

    if( touchWasDown ) {
        *outX = touchX;
        *outY = touchY;
        touchWasDown = false;
        
        return true;
        }
    
    return false;
    }





// wireless stuff
unsigned char wmBuffer[ WM_SYSTEM_BUF_SIZE ] ATTRIBUTE_ALIGN( 32 );

char wmShouldStop = true;

int wmStatus = -2;
char mpRunning = false;

// true for parent, false for child
char isParent = false;

// remains true until fist connection breaks
char isFirstConnection = true;


#define WM_DMA_NO  2

#include "ggid.h"

unsigned short tgid = 0;

unsigned short allowedChannels = 0;
unsigned short nextChannel = 0;



static WMParentParam parentParam ATTRIBUTE_ALIGN(32);

static unsigned short remoteAID = 0;

static unsigned short portNumber = 13;



// for child scanning
static WMScanParam scanParam;
static WMBssDesc scanBssDesc;

// FIXME:  add later to send parent specific data on connect
// NULL for now (all zeros sent)
//static unsigned char childSSID[ WM_SIZE_CHILD_SSID ];



static unsigned char *sendBuffer = NULL;
static unsigned char *receiveBuffer = NULL;


#include "DataFifo.h"


DataFifo sendFifo;
DataFifo receiveFifo;

char sendPending = false;



static void wmEndCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    printOut( "wmEndCallback received\n" );

    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndCallback\n" );
        }
    else {
        if( sendBuffer != NULL ) {
            delete [] sendBuffer;
            sendBuffer = NULL;
            }
        if( receiveBuffer != NULL ) {
            delete [] receiveBuffer;
            receiveBuffer = NULL;
            }

        sendFifo.clearData();
        receiveFifo.clearData();
        sendPending = false;
        }

    // first connection (in case of child) is over
    isFirstConnection = false;

    // don't hide reporting of an error (which leads to an End call)
    if( wmStatus != -1 ) {    
        // back to totally unconnected state
        wmStatus = -2;
        }
    
    }



static void wmDisconnectCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmDisconnectCallback\n" );
        }
    else {
        WM_End( wmEndCallback );
        }
    }


static void disconnect() {
    if( isParent ) {
        WM_Disconnect( wmDisconnectCallback, remoteAID );
        }
    else {
        // parent AID is 0
        WM_Disconnect( wmDisconnectCallback, 0 );
        }
    }



static void wmEndMPCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndMPCallback\n" );
        }
    else {
        mpRunning = false;
        //disconnect();
        }
    }








static void wmPortCallback( void *inArg ) {
    WMPortRecvCallback *callbackArg = (WMPortRecvCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error returned to wmPortCallback\n" );

        wmStatus = -1;
        WM_EndMP( wmEndMPCallback );
        }
    else {
        if( callbackArg->state == WM_STATECODE_PORT_RECV ) {
            //printOut( "wmPortCallback getting message of length %d\n",
            //          callbackArg->length );
            
            unsigned char channel = 0;
            
            if( callbackArg->length >= 5 ) {
                // room for channel flag
                channel = ( (unsigned char*)callbackArg->data )[4];
                }

            printOut( "Putting message of %d bytes onto Receive fifo, "
                      "chan %d\n", 
                      (unsigned int)callbackArg->length,
                      channel );

            receiveFifo.addData( (unsigned char*)callbackArg->data, 
                                 (unsigned int)callbackArg->length,
                                 channel );
            }
        }
    }



void startNextSend();

static void dummyC() {
    startNextSend();
    }

static void dummyB() {
    dummyC();
    }

static void dummyA() {
    dummyB();
    }


static void wmStartMPCallback( void *inArg ) {
    WMStartMPCallback *callbackArg = (WMStartMPCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {

        if( callbackArg->errcode == WM_ERRCODE_SEND_FAILED ) {
            printOut( "Send failed during MP, trying to continue\n" );
            }
        else if( callbackArg->errcode == WM_ERRCODE_TIMEOUT ) {
            printOut( "Timeout during MP, trying to continue\n" );
            }
        else {    
            wmStatus = -1;
            printOut( "Fatal error %d returned to wmStartMPCallback\n",
                      callbackArg->errcode );
            //disconnect();
            }
        }
    else {
        if( callbackArg->state == WM_STATECODE_MP_START ) {
            printOut( "MP Started\n" );
            
            wmStatus = 1;
            mpRunning = true;
            
            
            // prepare to receive data
            WMErrCode result = WM_SetPortCallback(
                portNumber, wmPortCallback, NULL );

            if( result !=  WM_ERRCODE_SUCCESS ) {
                wmStatus = -1;
                WM_End( wmEndCallback );
                }
            else {
                // send any data that is waiting to go
                //startNextSend();
                //dummyA();
                }
            
            }
        }
    }


static void startMP() {
    
    if( sendBuffer != NULL ) {
        delete [] sendBuffer;
        sendBuffer = NULL;
        }
    if( receiveBuffer != NULL ) {
        delete [] receiveBuffer;
        receiveBuffer = NULL;
        }

    // we clear these in wmEndCallback, but clear them here too, in 
    // case wmEnd wasn't called for some reason
    // don't ever want old, stale messages lingering in the FIFOs
    sendFifo.clearData();
    receiveFifo.clearData();
    sendPending = false;


    unsigned short sendBufferSize = 
        (unsigned short)WM_GetMPSendBufferSize();
    unsigned short receiveBufferSize = 
        (unsigned short)WM_GetMPReceiveBufferSize();
    printOut( "Send,receive buffer sizes are: %d,%d\n", 
              sendBufferSize,
              receiveBufferSize );
                 
    // make sure they are sane before allocating memory based
    // on them, because these sizes come from what the
    // parent sends
    if( sendBufferSize > 1920 || receiveBufferSize > 1920 ) {
        // bad sizes, bail out    
        wmStatus = -1;
        WM_End( wmEndCallback );
        return;
        }
                
    // sizes okay to allocate memory with
    // allocations are automatically 32-byte aligned
    sendBuffer = 
        new unsigned char[ sendBufferSize ];
    receiveBuffer = 
        new unsigned char[ receiveBufferSize ];

    // start multi port communications
    WMErrCode result =
        WM_StartMP(
            wmStartMPCallback,
            (u16*)receiveBuffer,
            receiveBufferSize,
            (u16*)sendBuffer,
            sendBufferSize,
            1 );  // one communication per frame
                
    if( result !=  WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        }

    }



static void wmSetEntryCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;

    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {

        wmStatus = -1;
        printOut( "Fatal error %d returned to wmSetEntryCallback\n",
                  callbackArg->errcode );
        }
    else {
        // success at blocking future child connection attempts
        
        printOut( "Child entry flag successfully disabled, starting MP\n" );
                
        // ready for MP
        startMP();
        }
    
    }


static void wmResetToEndCallback( void *inArg );



static void wmStartParentCallback( void *inArg ) {
    WMStartParentCallback *callbackArg = (WMStartParentCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmStartParentCallback: %d\n",
                  callbackArg->errcode );
        WM_End( wmEndCallback );
        }
    else {
        switch( callbackArg->state ) {
            case WM_STATECODE_PARENT_START: {
                
                
                }
                break;
            case WM_STATECODE_CONNECTED: {
                
                
                // don't start multiplayer until we're connected
                remoteAID = callbackArg->aid;
                printOut( "Parent sees child remote AID of %d\n", remoteAID );
                
                // stop accepting new child entries BEFORE we start MP
                printOut( "Disabling child entry flag now that we have one "
                          " child connected\n" );
                WM_SetEntry( wmSetEntryCallback, 0 );
                
                // FIXME:
                //parent_load_status();
                
                }
                return;
            case WM_STATECODE_DISCONNECTED:

                printOut( "Disconnected in wmStartParent\n" );
                
                // FIXME:
                //parent_load_status();
                    
                if( mpRunning ) {
                    mpRunning = false;
                    }
                printOut( "Shutting down network\n" );

                wmStatus = -1;
                WM_Reset( wmResetToEndCallback );
                
                return;
                    
            default:
                    
                return;
            }
        }
    }



    


static void wmSetParentParamCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmSetParentParamCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        WMErrCode result = WM_StartParent( wmStartParentCallback );
        
        if( result != WM_ERRCODE_OPERATING ) {
            wmStatus = -1;
            WM_End( wmEndCallback );
            }
        }
    
    }



static void measureChannelCallback( short inBestChannel ) {
    // done measuring

    if( inBestChannel == -1 ) {
        wmStatus = -1;
        WM_End( wmEndCallback );

        return;
        }
    else {

        // set parent params
        // FIXME:  use userGameInfo to specify identifying
        // info for child lobby, like parent name
        parentParam.userGameInfo = NULL;
        parentParam.userGameInfoLength = 0;
        parentParam.tgid = tgid;
        parentParam.ggid = LOCAL_GGID;
        parentParam.channel = (unsigned short)inBestChannel;
        parentParam.beaconPeriod = WM_GetDispersionBeaconPeriod();
        parentParam.parentMaxSize = 512;
        parentParam.childMaxSize = 512;
        parentParam.maxEntry = 1;
        parentParam.CS_Flag = 0;
        parentParam.multiBootFlag = 0;
        parentParam.entryFlag = 1;
        parentParam.KS_Flag = 0;
            
        WMErrCode result =
            WM_SetParentParameter( wmSetParentParamCallback,  
                                   &parentParam );
        if( result != WM_ERRCODE_OPERATING ) {
            wmStatus = -1;
            WM_End( wmEndCallback );
            }
        }
    }




static void wmStartConnectCallback( void *inArg );


// connect to the game that we found
static void startConnect() {
    WMErrCode result = WM_StartConnect(
        wmStartConnectCallback,
        &scanBssDesc,
        // FIXME:
        // can specify a 24-byte SSID here that parent
        // can check... parent not checking, so NULL for now
        // which forces all zeros as SSID
        NULL );
    
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        printOut( "Error on startConnect\n" );
        WM_End( wmEndCallback );
        }
    }



static void wmResetCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmResetCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // reset only called, with this callback, from child

        // start connection process again to same parent
        printOut( "Trying to connect to parent again after reset\n" );
        startConnect();
        }
    
    }



void scanNextChannel();


static void wmResetToRescanCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmResetToRescanCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // reset only called, with this callback, from child

        // start connection process again to same parent
        printOut( "Trying to scan for parent again after reset\n" );
        scanNextChannel();
        }
    
    }


static void wmResetToEndCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    printOut( "wmResetToEndCallback received\n" );
    

    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmResetToEndCallback\n" );
        
        }

    printOut( "Trying to call WM_End\n" );
    WMErrCode result = WM_End( wmEndCallback );

    if( result != WM_ERRCODE_OPERATING ) {
        printOut( "Error code on WM_End = %d\n", result );
        }
    }



static void wmStartConnectCallback( void *inArg ) {
    WMStartConnectCallback *callbackArg = (WMStartConnectCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        // no longer a fatal error
        //wmStatus = -1;
        printOut( "Error returned to wmStartConnectCallback: %d\n",
                  callbackArg->errcode );

        if( !wmShouldStop ) {
            if( callbackArg->errcode == WM_ERRCODE_NO_ENTRY 
                ||
                callbackArg->errcode == WM_ERRCODE_OVER_MAX_ENTRY ) {
                printOut( "Parent that we're trying to connect to is already "
                          "full.  Resetting and scanning for a "
                          "different one\n" );
                WM_Reset( wmResetToRescanCallback );
                }
            else if( callbackArg->errcode == WM_ERRCODE_FAILED ) {
                printOut( "Connection to chosen Parent failed completely.  "
                          "Resetting and scanning for a "
                          "different one\n" );
                WM_Reset( wmResetToRescanCallback );
                }
            else {
                printOut( "Non fatal error when connecting to parent?"
                          "Trying to reset and reconnect to same parent\n" );
                WM_Reset( wmResetCallback );
                }
            }
        else {
            printOut( "Should stop anyway, resetting to end\n" );
            WM_Reset( wmResetToEndCallback );
            }
        }
    else {
        switch( callbackArg->state ) {
            case WM_STATECODE_CONNECTED: {
                
                startMP();
                
                //aid = cb->aid;
                }
        
                break;

            case WM_STATECODE_BEACON_LOST:
                printOut( "Beacon lost in wmStartConnect\n" );

                //__callback(CHILD_BEACONLOST, 0);
                
                break;
                
            case WM_STATECODE_DISCONNECTED:
                // flag MP as not running here
                mpRunning = false;
                
                printOut( "Disconnected in wmStartConnect\n" );
                
                printOut( "Shutting down network\n" );

                wmStatus = -1;
                WM_Reset( wmResetToEndCallback );


                /*
                if( !wmShouldStop ) {
                    printOut( "Trying to connect again\n" );
                
                    printOut( "Resetting connection\n" );
                    
                    WM_Reset( wmResetCallback );
                    }
                else {
                    printOut( "Should stop anyway, resetting to end\n" );
                    WM_Reset( wmResetToEndCallback );
                    }
                */
                //__callback(CHILD_DISCONNECTED, 0);
                
                break;
                
            default:
                break;
            }
        }
    }



static void wmEndScanCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmEndScanCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        // ending scan only when we found a connection

        // use this game
                
        startConnect();
        

        }
    }





static void wmStartScanCallback( void *inArg ) {
    WMStartScanCallback *callbackArg = (WMStartScanCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmStartScanCallback\n" );
        WM_End( wmEndCallback );
        }
    else {
        if( callbackArg->state == WM_STATECODE_PARENT_FOUND ) {
            if( callbackArg->gameInfo.ggid == LOCAL_GGID 
                &&
                // parent allowing entry
                ( callbackArg->gameInfo.attribute & WM_ATTR_FLAG_ENTRY ) != 0
                &&
                // parent NOT a multiboot parent
                // we don't want to connect to one of these by accident here
                ( callbackArg->gameInfo.attribute & WM_ATTR_FLAG_MB ) == 0 ) {
 
                printOut( "Parent matches our GGID, is not MB, and is "
                          "allowing entry, ending scan\n" );
                

                // use the first parent that we find (ignore rest)
                
                // end our scan before connecting
                WMErrCode result = WM_EndScan( wmEndScanCallback );
                                
                if( result != WM_ERRCODE_OPERATING ) {
                    wmStatus = -1;
                    WM_End( wmEndCallback );
                    }
                
                // Actually, if end goal is Download play only (cloneboot)
                // then these may not be relevant (one known parent, no lobby)

                // FIXME:  in future, should present a list of parents to
                // user and let them pick (lobby interface)

                // FIXME:  don't forget to use callbackArg->linkLevel
                // to display official wifi level icon in lobby interface
                return;
                }
            }

        // parent not found or ggid didn't match

        nextChannel++;
        
        if( nextChannel >= 13 ) {
            nextChannel = 0;
            }

        if( !wmShouldStop ) {
            // make sure we don't keep scanning loop going if we've been
            // asked to stop
            scanNextChannel();
            }
        }
    }

        






void scanNextChannel() {
    if( allowedChannels == 0 ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        return;
        }
    while( ! ( (1 << nextChannel) & allowedChannels ) ) {
        nextChannel++;
        
        // 14 broken?
        // wrap back around
        if( nextChannel >= 13 ) {
            nextChannel = 0;
            }
        }
    

    scanParam.channel = (unsigned short)( nextChannel + 1 );

    DC_InvalidateRange( &scanBssDesc, sizeof(WMBssDesc) );
    

    //printOut( "Calling WM_StartScan for channel %d\n", nextChannel + 1 );
    
    WMErrCode result = 
        WM_StartScan( wmStartScanCallback, 
                      &scanParam );
    
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        WM_End( wmEndCallback );
        }
    }




static void wmPortSendCallback( void *inArg ) {
    WMPortSendCallback *callbackArg = (WMPortSendCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        wmStatus = -1;
        printOut( "Error returned to wmPortSendCallback\n" );
        WM_EndMP( wmEndMPCallback );
        }
    else {

        if( callbackArg->sentBitmap == callbackArg->destBitmap ) {
            
            printOut( "Data send successful\n" );

            unsigned char channel = 0;
            char lastMessageFlag = false;

            if( callbackArg->length >= 6 ) {
                // room for flags
                channel = ( (unsigned char*)callbackArg->data )[4];
                lastMessageFlag = 
                    (char)( ( (unsigned char*)callbackArg->data )[5] );
                }

            delete [] callbackArg->data;
            sendPending = false;
            
            if( lastMessageFlag ) {
                printOut( "Last message sent.  Auto-closing connection.\n" );
                closeConnection();
                }

            // next send will start during next network step
            }
        else {
            printOut( "Data send did not reach all recipients\n" );

            unsigned char channel = 0;
            char lastMessageFlag = false;

            if( callbackArg->length >= 6 ) {
                // room for flags
                channel = ( (unsigned char*)callbackArg->data )[4];
                lastMessageFlag = 
                    (char)( ( (unsigned char*)callbackArg->data )[5] );
                }

            // put back on send fifo in next position to try again
            sendFifo.pushData( (unsigned char *)( callbackArg->data ), 
                               callbackArg->length, channel, lastMessageFlag );
            sendPending = false;
            
            delete [] callbackArg->data;

            if( !mpRunning ) {
                printOut( "Will try resending when MP starts again\n" );
                }
            else {
                printOut( "MP still running, trying to send again\n" );
                // next send during next network step
                }                
            }
        
        }
    }



void startNextSend() {
    
    unsigned int numBytes;
    unsigned char channel;
    
    // ignore channels, just send next message
    unsigned char *data = sendFifo.getData( &numBytes, false, 0, &channel );
    
    if( data != NULL ) {
        sendPending = true;
        
        printOut( "Starting data send of %d bytes\n", numBytes );
        
        unsigned short receiverBitmap = (unsigned short)( 1 << remoteAID );
        
        
        WMErrCode result = WM_SetMPDataToPort(
            wmPortSendCallback,
            (unsigned short *)data,
            (unsigned short)numBytes ,
            receiverBitmap,
            portNumber,
            0 );

        if( result == WM_ERRCODE_ILLEGAL_STATE ) {
            printOut( "Trying to send from illegal state\n" );
            
            printOut( "Pushing data back onto send fifo until later\n" );
            
            unsigned char channel = 0;
            char lastMessageFlag = false;
            
            if( numBytes >= 6 ) {
                // room for flags
                channel = data[4];
                lastMessageFlag = (char)( data[5] );
                }

            sendFifo.pushData( data, numBytes, channel, lastMessageFlag );

            sendPending = false;
            
            delete [] data;
            
            // what state are we in?
            WMStatus status;
            WM_ReadStatus( &status );
            
            if( status.state == WM_STATE_PARENT ||
                status.state == WM_STATE_CHILD ) {
                
                printOut( "During send:  "
                          "no longer in MP state\n" );
                
                printOut( "Will retry send after MP restarts\n" );
                
                mpRunning = false;
                
                // don't restart MP here... assume it will be restarted
                // elsewhere upon reconnect
                // startMP();
                }
            
                
            }
        
        }
    else{
        sendPending = false;
        }
    }




static void wmInitializeCallback( void *inArg ) {
    WMCallback *callbackArg = (WMCallback *)inArg;
    
    if( callbackArg->errcode != WM_ERRCODE_SUCCESS ) {
        printOut( "Error in wmInitialize\n" );
        wmStatus = -1;
        }
    else {
        
        allowedChannels = WM_GetAllowedChannel();
        nextChannel = 0;

        if( isParent ) {
            // can't call this from inside a callback
            //tgid = WM_GetNextTgid();

            // only 5 traffic units used by regular game
            // But game can survive (establish connection, exchange moves)
            //  with as few as 3 free traffic units.
            // (measured with WMtest tool)
            startMeasureChannel( measureChannelCallback, 3 );
            }
        else {
            // child
            
            // scan for parent's beacon
            
            // Scan for any MAC address
            scanParam.bssid[0] = 0xFF;
            scanParam.bssid[1] = 0xFF;
            scanParam.bssid[2] = 0xFF;
            scanParam.bssid[3] = 0xFF;
            scanParam.bssid[4] = 0xFF;
            scanParam.bssid[5] = 0xFF;
            
            scanParam.maxChannelTime = WM_GetDispersionScanPeriod();
            scanParam.scanBuf = &scanBssDesc;
            

            nextChannel = 0;


            // only scan just for parent, as clone child, the first time
            if( !MB_IsMultiBootChild() || !isAutoconnecting() ) {
                printOut( "Child starting first scanNextChannel() call\n" );
                scanNextChannel();
                }
            else{
                printOut( "CloneBoot child Child reconnecting to parent "
                          "by scanning only for that parent\n" );
                
                const MBParentBssDesc* parentBssDesc = 
                    MB_GetMultiBootParentBssDesc();
                
                *(u16 *)(&scanParam.bssid[0]) = parentBssDesc->bssid[0];

                *(u16 *)(&scanParam.bssid[2]) = parentBssDesc->bssid[1];

                *(u16 *)(&scanParam.bssid[4]) = parentBssDesc->bssid[2];
                
                
                scanNextChannel();

                /*
                  typedef	struct
                  {
                  u16         length;
                  u16         rssi;
                  u16    bssid[3];
                  u16         ssidLength;
                  u8              ssid[32];
                  u16         capaInfo;
                  struct
                  {
                  u16     basic;
                  u16     support;
                  } rateSet;
                  u16         beaconPeriod;
                  u16         dtimPeriod;
                  u16         channel;
                  u16         cfpPeriod;
                  u16         cfpMaxDuration;
                  } MBParentBssDesc;

                */
                
                /*
                  typedef struct WMBssDesc
                  {
                  u16         length;
                  u16         rssi;
                  u8          bssid[ WM_SIZE_BSSID ];
                  u16         ssidLength;
                  u8          ssid[WM_SIZE_SSID];
                  u16         capaInfo;
                  struct
                  {
                  u16     basic;
                  u16     support;
                  } rateSet;
                  u16         beaconPeriod;
                  u16         dtimPeriod;
                  u16         channel;
                  u16         cfpPeriod;
                  u16         cfpMaxDuration;
                  u16         gameInfoLength;
                  u16         rsv;
                  WMGameInfo  gameInfo;
                  } WMBssDesc;
                */
                
                /*
                // copy the first part into our WMBssDesc
                memcpy( &scanBssDesc, parentBssDesc, 
                        sizeof( MBParentBssDesc ) );
                
                startConnect();
                */
                }                    
            
            }        
        }
    }



static void initWM() {
    
    WMErrCode result = WM_Initialize( &wmBuffer, wmInitializeCallback, 
                                      WM_DMA_NO );
    if( result != WM_ERRCODE_OPERATING ) {
        wmStatus = -1;
        }
    }



int getSignalStrength() {
    return WM_GetLinkLevel();
    }



char isAutoconnecting() {
    // only auto-connect to parent for first connection
    
    // after that, child can host games with others, or join others

    return isCloneChild && isFirstConnection;
    }



char shouldShowDSWiFiIcons() {
    return true;
    }



char *getLocalAddress() {
    return NULL;
    }


void acceptConnection() {
    wmShouldStop = false;
    wmStatus = 0;
    
    isParent = true;

    printOut( "Getting TGID\n" );
    
    tgid = WM_GetNextTgid();
    
    printOut( "Tgid = %d\n", (int)tgid );

    
    initWM();
    }


void connectToServer( char *inAddress ) {
    wmShouldStop = false;
    wmStatus = 0;

    // ignore address
    // prevent compiler warnings for unused
    inAddress = NULL;


    isParent = false;
    initWM();
    }


int checkConnectionStatus() {
    return wmStatus;
    }


void closeConnection() {
    printOut( "closeConnection() called\n" );
    
    wmShouldStop = true;


    // FIXME:  need to call specific WM_End_____ call to start wind-down
    // depending on what state we're in (see wireless_package sample code)

    // Or maybe WM_Reset will do the trick.... takes us to IDLE, from
    // which we can call the generic WM_End right away.

    printOut( "Trying to reset to end\n" );
    WMErrCode result = WM_Reset( wmResetToEndCallback );
    if( result != WM_ERRCODE_OPERATING ) {
        printOut( "Error code on reset = %d\n", result );
        }
    }


static void sendMessage( unsigned char *inMessage, unsigned int inLength,
                         unsigned char inChannel, char inLastMessageFlag ) {

    if( wmStatus != 1 ) {
        printOut( "sendMessage called when WM network not running.  "
                  "Discarding message\n" );
        return;
        }
    
    printOut( "Putting message of %d bytes onto send fifo, "
              "channel %d\n", inLength, inChannel );
    
    // add first 4 bytes, which is message length
    // must do this because DS data sharing doesn't seem to preserve
    // data length (sometimes longer than message that was sent, with
    // garbage bytes at end).
    // also add channel field
    // also add last message flag field
    unsigned int sendSize = inLength + 4 + 1 + 1;
    unsigned char *sendData = new unsigned char[ sendSize ];
                
    sendData[0] = (unsigned char)( ( inLength >> 24 ) & 0xFF );
    sendData[1] = (unsigned char)( ( inLength >> 16 ) & 0xFF );
    sendData[2] = (unsigned char)( ( inLength >> 8 ) & 0xFF );
    sendData[3] = (unsigned char)( ( inLength ) & 0xFF );
    
    sendData[4] = inChannel;
    sendData[5] = (unsigned char)inLastMessageFlag;

    memcpy( &( sendData[6] ), inMessage, inLength );


    sendFifo.addData( sendData, sendSize, inChannel, inLastMessageFlag );
    delete [] sendData;
    
    // send will start during next network step
    }


void sendMessage( unsigned char *inMessage, unsigned int inLength,
                  unsigned char inChannel ) {
    sendMessage( inMessage, inLength, inChannel, false );
    }


void sendLastMessage( unsigned char *inMessage, unsigned int inLength,
                      unsigned char inChannel ) {
    sendMessage( inMessage, inLength, inChannel, true );
    }



unsigned char *getMessage( unsigned int *outLength, unsigned char inChannel ) {
    unsigned int dataLength;
    unsigned char channel;
    
    unsigned char *data = receiveFifo.getData( &dataLength, true, inChannel,
                                               &channel );
    
    if( data == NULL ) {
        return NULL;
        }
    

    // else decode the length header at start of message

    if( dataLength < 4 ) {
        printOut( "getMessage Error:  message too short for length header\n" );
        delete [] data;
        return NULL;
        }
    
    unsigned int messageLength =
        (unsigned int)( data[0] << 24 ) |
        (unsigned int)( data[1] << 16 ) |
        (unsigned int)( data[2] << 8 ) |
        (unsigned int)( data[3] );

    // make sure size is sane
    if( messageLength > 10000000 ) {
        printOut( "Huge message size of %u received\n",
                  messageLength );
        delete [] data;
        return NULL;
        } 
    
    /*
    if( dataLength > messageLength + 5 ) {
        printOut( "Extra bytes padded onto end of received message:%d\n",
                  dataLength - ( messageLength + 5 ) );
        }
    */

    if( data[4] != inChannel ) {
        printOut( "Message's channel field %u does not match flag in data "
                  "fifo %u", data[4], inChannel );
        delete [] data;
        return NULL;
        }

  //printOut( "Message of length %d waiting in receive FIFO for getMessage\n", 
  //            messageLength );
    
    unsigned char *message = new unsigned char[ messageLength ];
    memcpy( message, &( data[6] ), messageLength );
    
    delete [] data;
    
    *outLength = messageLength;
    
    return message;
    }



void stepNetwork() {
    if( !sendPending && mpRunning ) {
        // try sending next message
        startNextSend();
        }
    }




//FIXME:  camera stuff

char isCameraSupported() {
    if( OS_IsRunOnTwl() ) {
        char exchangeRestricted = false;
        
#ifdef SDK_TWL
        exchangeRestricted = (char)OS_IsRestrictPhotoExchange();
#endif
        if( !exchangeRestricted ) {
            printOut( "Camera supported!\n" );
            return true;
            }
        }

    printOut( "Camera not supported!\n" );
    return false;
    }



static unsigned short *cameraFrameBuffer[2] = { NULL, NULL };
int writingToBuffer = 0;
#define CAM_DMA_NO 1


#ifdef SDK_TWL

static void CameraDmaRecvCallback( void* inArg) {
#pragma unused( inArg )

    MI_StopNDma( CAM_DMA_NO );
    
    printOut( "Cam receive callback\n" );
    
    if( CAMERA_IsBusy() == TRUE ) {
        // OS_TPrintf(".");

        printOut( "Cam busy (good)\n" );
        // Check whether image transfer is complete
        if( MI_IsNDmaBusy( CAM_DMA_NO ) ) {
            //OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma( CAM_DMA_NO );
            }

        writingToBuffer = ( writingToBuffer + 1 ) % 2;
        
        CAMERA_DmaRecvAsync( CAM_DMA_NO, cameraFrameBuffer[ writingToBuffer ], 
                             CAMERA_GetBytesAtOnce( CAM_W ), 
                             CAMERA_GET_FRAME_BYTES( CAM_W, CAM_H), 
                                 CameraDmaRecvCallback, NULL );
        
        }
    }
#endif



// start producing frames
void startCamera() {

#ifdef SDK_TWL
    CAMERAResult result = CAMERA_Init();
    if( result != CAMERA_RESULT_SUCCESS ) {
        printOut( "Init camera failed.\n" );
        return;
        }

    // 160x120
    result = CAMERA_I2CSize( CAMERA_SELECT_IN, CAMERA_SIZE_QQVGA );
    
    if( result != CAMERA_RESULT_SUCCESS ) {
        printOut( "Setting camera size failed.\n" );
        return;
        }
    CAMERA_SetOutputFormat( CAMERA_OUTPUT_RGB );
    
    CAMERA_SetTransferLines( CAMERA_GET_MAX_LINES( CAM_W ) ); 

    result = CAMERA_Start( CAMERA_SELECT_IN );
    
    if( result != CAMERA_RESULT_SUCCESS ) {
        printOut( "Starting camera failed.\n" );
        return;
        }

    for( int b=0; b<2; b++ ) {

        cameraFrameBuffer[b] = 
            new unsigned short[ CAMERA_GET_FRAME_BYTES( CAM_W, CAM_H ) / 2 ];
        memset( cameraFrameBuffer[b], 0, 
                CAMERA_GET_FRAME_BYTES( CAM_W, CAM_H ) );
        }
    
    writingToBuffer = 0;

    CAMERA_DmaRecvAsync( CAM_DMA_NO, cameraFrameBuffer[writingToBuffer], 
                         CAMERA_GetBytesAtOnce( CAM_W), 
                         CAMERA_GET_FRAME_BYTES( CAM_W, CAM_H), 
                         CameraDmaRecvCallback, NULL );
#endif
    }


// stop producing frames
void stopCamera() {

#ifdef SDK_TWL
    CAMERAResult result;
    result = CAMERA_Stop();

    for( int b=0; b<2; b++ ) {
        if( cameraFrameBuffer[b] != NULL ) {
            delete [] cameraFrameBuffer[b];
            cameraFrameBuffer[b] = NULL;
            }
        }
    
    if( result != CAMERA_RESULT_SUCCESS ) {
        printOut( "Stopping camera failed.\n" );
        return;
        }

    CAMERA_End();
#endif
    }



// Trimming size fixed at 160x120
// format fixed at grayscale 256 levels

// get the next frame
// inBuffer is where 160x120 grayscale pixels will be returned
void getFrame( unsigned char *inBuffer ) {

    
    int numPixels = CAM_W * CAM_H;

    unsigned short sumMax = 31 * 3;

    // read from buffer not being written to
    int readFromBuffer = ( writingToBuffer + 1 ) % 2;
    
    unsigned short *readBuffer = cameraFrameBuffer[ readFromBuffer ];
    
    
    for( int i=0; i<numPixels; i ++ ) {
        unsigned short pixel = readBuffer[ i ];
                
        int r = ( pixel & 0x001F );
        int g = ( (pixel >>  5) & 0x001F );
        int b = ( (pixel >> 10) & 0x001F );
                
        int sum = r + g + b;
                
        unsigned char gray = (unsigned char)( (sum * 255) / sumMax );
        //unsigned char gray = (unsigned char)( r << 3 );
        
        
        inBuffer[ i ] = gray;
        }
    }


// snap the next frame as a finished picture
void snapPicture( unsigned char *inBuffer ) {
    
    getFrame( inBuffer );
    
    // FIXME:  play Shutter sound here
    }













// stuff for drawing 3D on both screens (one screen each vblank)

char shouldSwap = false;
char isEvenFrame = false;

GXOamAttr oamAttr[ 128 ];

// thread that waits for a safe time to swap
#define SWAP_THREAD_STACK_SIZE  1024
#define SWAP_THREAD_PRIORITY  ( OS_THREAD_LAUNCHER_PRIORITY - 6 )

OSThread swapThread;
unsigned int swapThreadStack[ SWAP_THREAD_STACK_SIZE /
                              sizeof( unsigned int ) ];
void swapThreadProcess( void *inData );



// functions called by thread to switch drawing modes between frames


static void setupEvenFrame() {

    GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

    GX_ResetBankForSubOBJ();
    GX_SetBankForSubBG( GX_VRAM_SUB_BG_128_C );
    GX_SetBankForLCDC( GX_VRAM_LCDC_D);

    GX_SetCapture( GX_CAPTURE_SIZE_256x192,
                   GX_CAPTURE_MODE_A,
                   GX_CAPTURE_SRCA_2D3D, 
                   (GXCaptureSrcB)0, 
                   GX_CAPTURE_DEST_VRAM_D_0x00000, 16, 0 );

    GX_SetGraphicsMode( GX_DISPMODE_VRAM_D, GX_BGMODE_0, GX_BG0_AS_3D ); 
    
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
    G2_SetBG0Priority( 0 );

    GXS_SetGraphicsMode( GX_BGMODE_5 );
    GXS_SetVisiblePlane( GX_PLANEMASK_BG2 );
    
    G2S_SetBG2ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256,
                            GX_BG_AREAOVER_XLU, 
                            GX_BG_BMPSCRBASE_0x00000 );
    
    G2S_SetBG2Priority( 0 );
    G2S_BG2Mosaic( FALSE );
    }


static void setupOddFrame() {
    GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
    GX_ResetBankForSubBG();
    GX_SetBankForSubOBJ( GX_VRAM_SUB_OBJ_128_D );
    GX_SetBankForLCDC( GX_VRAM_LCDC_C );

    GX_SetCapture( GX_CAPTURE_SIZE_256x192,
                   GX_CAPTURE_MODE_A,
                   GX_CAPTURE_SRCA_2D3D,
                   (GXCaptureSrcB)0, 
                   GX_CAPTURE_DEST_VRAM_C_0x00000, 16, 0);

    
    GX_SetGraphicsMode( GX_DISPMODE_VRAM_C, GX_BGMODE_0, GX_BG0_AS_3D );

    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
    G2_SetBG0Priority( 0 );

    GXS_SetGraphicsMode( GX_BGMODE_5 );
    GXS_SetVisiblePlane( GX_PLANEMASK_OBJ );
    }



void swapThreadProcess( void *inData ) {
    // not using, compiler warns us
    void *data = inData;
    
    while( true ) {

        if( GX_GetVCount() <= 193 ) {
#ifdef SDK_TWL
            OS_SpinWaitSysCycles( 784 );
#else
            OS_SpinWait( 784 );
#endif
            }
        if( !G3X_IsGeometryBusy() && shouldSwap ) {
            if( isEvenFrame ) {
                setupEvenFrame();
                }
            else {
                setupOddFrame();
                }
            shouldSwap = false;
            isEvenFrame = !isEvenFrame;
            }
        
        OS_SleepThread( NULL );    
        }
    }

    

static void VBlankCallback() {
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
    
    // wake thread up every vblank
    OS_WakeupThreadDirect( &swapThread );
    }



// sound data structures


#define SOUND_BUFFER_PAGESIZE 1024 * 32
//#define SOUND_BUFFER_PAGESIZE 1024 * 32
// each buffer has two pages for double-buffering
#define SOUND_BUFFER_SIZE SOUND_BUFFER_PAGESIZE * 2


static s16 soundBuffer[MAX_SOUND_CHANNELS]
                      [SOUND_BUFFER_SIZE] ATTRIBUTE_ALIGN(32);
static int soundBufferPage[MAX_SOUND_CHANNELS];


int soundSampleRate = SOUND_SAMPLE_RATE;
int soundTimerValue = SND_TIMER_CLOCK / soundSampleRate;
u32 soundAlarmPeriod = soundTimerValue * SOUND_BUFFER_PAGESIZE / 32U;




// we can't fill sound buffers right in the Alarm callback, because
// it bogs down the interrupt handler (and even causes screen artifacts!)
// so, we need to pass buffer processing off to a thread that can
// run separately from the interrupt handler

// larger stack for deeper function calls, like opening files 
#define SOUND_THREAD_STACK_SIZE  2048
//#define SOUND_THREAD_PRIORITY  12
// our main thread runs at 16, and it gets starved by sound thread
// never want our FPS to drop because of sound.
#define SOUND_THREAD_PRIORITY  17

OSThread soundThread;
unsigned int soundThreadStack[ SOUND_THREAD_STACK_SIZE /
                               sizeof( unsigned int ) ];

static char soundTryingToRun = false;


char isSoundTryingToRun() {
    return soundTryingToRun;
    }


static OSMessageQueue soundMessageQueue;
static OSMessage soundMessageBuffer[1];

static OSMutex soundMutex;


int soundThreadCallCount = 0;


static void soundThreadProcess( void *inData ) {
    
    inData = NULL;
    

    OSMessage message;

    OS_InitMessageQueue( &soundMessageQueue, soundMessageBuffer, 1 );

    while( true ) {
        // wait for a message from the Alarm callback
        soundTryingToRun = false;
        OS_ReceiveMessage( &soundMessageQueue, &message, OS_MESSAGE_BLOCK );
        
        lockAudio();
        
        for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
            s16 *buffer = soundBuffer[i];
        
            if( soundBufferPage[i] == 1 ) {
                // jump int buffer to second page
                buffer = &( buffer[ SOUND_BUFFER_PAGESIZE ] );
                }
            
            getAudioSamplesForChannel( i, buffer, SOUND_BUFFER_PAGESIZE );
            }
        
        soundThreadCallCount++;
        
        unlockAudio();
        }
    }





static int soundAlarmCount = 0;

static void SoundAlarmCallback( void *inArg ) {
    
    // switch pages every time alarm is called, not every time thread runs
    // if we switch pages in thread, we can fall behind after a slow thread
    // run and be out of sync with the page we should actually be writing
    // into, leading to persistent glitches

    // do this for all but the first alarm (when we should be writing into
    // page 0)
    if( soundAlarmCount != 0 ) {
        
        for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {
            
            if( soundBufferPage[i] == 0 ) {
                soundBufferPage[i] = 1;
                }
            else if( soundBufferPage[i] == 1 ) {
                soundBufferPage[i] = 0;
                }
            }
        }

    // printOut( "Sound Alarm called %d\n", soundAlarmCount );
    soundAlarmCount ++;
    
    soundTryingToRun = true;
    OS_SendMessage( &soundMessageQueue, (OSMessage)inArg, OS_MESSAGE_NOBLOCK );
    }




static char shouldDrawNintendoLogo = false;
static unsigned char nintendoLogoFade = 0;
static int nintendoLogoSpriteID = -1;
static int indiePubLogoSpriteID = -1;

static int skipLogo = false;

static char skippingLogo() {
    if( skipLogo ) {
        return true;
        }
    
    int x,y;
    if( getTouch( &x, &y ) || 
        PAD_Read() != 0 ) {
        
        skipLogo = true;
        return true;
        }

    return false;
    }




#ifdef SDK_TWL
    void TwlMain( void )    
#else
    void NitroMain( void )
#endif
{
    
    
    // already called in NitroStartUp
    //OS_Init();
    
    OS_Printf( "Main starting\n" );

    RTC_Init();
    OS_InitTick();
    OS_InitAlarm();

    FX_Init();
    TP_Init();
    

    // now implemented in common code
    //MATH_InitRand32( &randContext, 13728749 );
    
    
    
    
    GX_Init();

    // DMA is not used in GX (the old DMA conflicts with camera DMA)
    //(void)GX_SetDefaultDMA( GX_DMA_NOT_USE );

    
    GX_DispOff();
    GXS_DispOff();



    // turn on VBLANK interrupt handling
    OS_SetIrqFunction( OS_IE_V_BLANK, VBlankCallback );
    OS_EnableIrqMask( OS_IE_V_BLANK );

    GX_VBlankIntr( true );  

    OS_EnableIrq();
    OS_EnableInterrupts();


    // init vram
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    MI_CpuClearFast( (void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    
    GX_DisableBankForLCDC();

    MI_CpuFillFast( (void *)HW_OAM, 192, HW_OAM_SIZE );
    MI_CpuClearFast( (void *)HW_PLTT, HW_PLTT_SIZE );

    MI_CpuFillFast( (void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE );
    MI_CpuClearFast( (void *)HW_DB_PLTT, HW_DB_PLTT_SIZE );  


    // setup our thread to do draw mode swapping
    OS_InitThread();
    OS_CreateThread( &swapThread, swapThreadProcess, NULL, 
                     swapThreadStack + 
                     SWAP_THREAD_STACK_SIZE / sizeof(unsigned int), 
                     SWAP_THREAD_STACK_SIZE,
                     SWAP_THREAD_PRIORITY );
    OS_WakeupThreadDirect( &swapThread );


#ifdef SDK_TWL
    // needed by camera
    if( isCameraSupported() ) {
        MI_InitNDmaConfig();
        OS_EnableIrqMask( OS_IE_NDMA1 );
        }
#endif

    SND_Init();
    

    
    G3X_Init();
    G3X_InitTable();
    G3X_InitMtxStack();

    // GX_SetBankForTex( GX_VRAM_TEX_0_A );
    // 256K for texture data
    GX_SetBankForTex( GX_VRAM_TEX_01_AB );

    // 64K for texture palettes
    GX_SetBankForTexPltt( GX_VRAM_TEXPLTT_0123_E ); 


    G3X_AntiAlias( TRUE );
    G3X_AlphaBlend( TRUE );
    


    // setup OAM stuff for lower screen
    GXS_SetOBJVRamModeBmp( GX_OBJVRAMMODE_BMP_2D_W256 );

    for( int i=0; i<128; i++ ) {
        oamAttr[i].attr01 = 0;
        oamAttr[i].attr23 = 0;
        }
    
    int oamID = 0;
    
    for( int y=0; y<192; y+=64) {
        for( int x=0; x<256; x+=64 ) {
            G2_SetOBJAttr( &oamAttr[ oamID ],
                           x,
                           y,
                           0,
                           GX_OAM_MODE_BITMAPOBJ,
                           FALSE,
                           GX_OAM_EFFECT_NONE,
                           GX_OAM_SHAPE_64x64,
                           GX_OAM_COLOR_16, 
                           (y / 8) * 32 + (x / 8), 
                           15, 
                           0 );
            oamID++;
            }
        }

    DC_FlushRange( &oamAttr[0], sizeof(oamAttr) );

    GXS_LoadOAM( &oamAttr[0], 0, sizeof(oamAttr) );
    


    
    // setup 3D
    //G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3_SwapBuffers( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    
    // Forcing A_b to max prevents polygon's A values from affecting 
    // the pixel buffer's A values, eliminating weird effects when
    // translucent polygons overlap
    G3X_SetClearColor( GX_RGB( 0, 0, 0 ),
                       31,  // A_b remains opaque no matter what is drawn
                       0x7fff,
                       63,
                       FALSE );

    G3_ViewPort( 0, 0, 255, 191 );

    G3_Ortho(
        0, 3 * FX32_ONE,
        0, 4 * FX32_ONE,
        0,
        8 * FX32_ONE,
        NULL );

    G3_StoreMtx( 0 );


    // 2d/3d blending
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0,
                      GX_BLEND_PLANEMASK_BG0 |
                      GX_BLEND_PLANEMASK_BG1 |
                      GX_BLEND_PLANEMASK_BG2 |
                      GX_BLEND_PLANEMASK_BG3 |
                      GX_BLEND_PLANEMASK_OBJ |
                      GX_BLEND_PLANEMASK_BD,
                      16,
                      0 );



    

    OS_Printf( "Init file system\n" );
    FS_Init( 3 );
    // DMA conflicting with camera?
    //FS_Init( FS_DMA_NOT_USE );



    // test file system
    int fileSize;
    unsigned char *fileContents = readFile( "test.txt", &fileSize );
    
    if( fileContents != NULL ) {
        for( int i=0; i<fileSize; i++ ) {
            OS_Printf( "%c", fileContents[ i ] );
            }
        delete [] fileContents;
        }
    else {
        OS_Printf( "File read failed\n" );
        }



    
    
    // start display
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();



    printOut( "Calibrating touch panel\n" );
    
    TPCalibrateParam tpCalibrate;

    if( !TP_GetUserInfo( &tpCalibrate ) ) {
        // don't panic here.
        // possible to go on with uncalibrated values
        printOut( "Failed to read touch panel calibration\n" );
        }
    else {
        TP_SetCalibrateParam( &tpCalibrate );
        }
    
    TP_SetCallback( autoTouchCallback );

    // start auto-sampling touch panel so we don't miss touch values
    // that happen during slow frames (like when the AI is running at full
    // CPU)
    TP_RequestAutoSamplingStart( 0, 4, tpAutoSampleBuffer, 
                                 TP_SAMPLE_BUFFER_SIZE );




    // can't call isCloneChild yet because it hasn't been inited yet
    // (inited during gameInit, below)
    if( ! MB_IsMultiBootChild() ) {
        // display Licensed By Nintendo logo, as required

        // indiePub logo on bottom screen at the same time
        
        nintendoLogoSpriteID = loadSprite( "nintendoLogo.tga", false );
        indiePubLogoSpriteID = loadSprite( "indiePubLogo.tga", false );
        
        shouldDrawNintendoLogo = true;
        
        // fade in for 0.25 seconds, ~8 frames

        // don't allow skipping during fade-in, to avoid not showing logo
        // at all if user holds down a button from the IPL screen onward.
        while( nintendoLogoFade < 255 ) {
            
            runGameLoopOnce();
            runGameLoopOnce();
            
            if( nintendoLogoFade < 224 ) {
                nintendoLogoFade += 32;
                }
            else {
                nintendoLogoFade = 255;
                }
            }
        
        // hold logo for 1 second, 30 frames
        for( int f=0; f<30 && !skippingLogo(); f++ ) {
            runGameLoopOnce();
            runGameLoopOnce();
            }
        
        // fade back out for 0.25 seconds
        while( nintendoLogoFade > 0 ) {
            
            
            if( nintendoLogoFade > 31 ) {
                nintendoLogoFade -= 32;
                }
            else {
                nintendoLogoFade = 0;
                }

            runGameLoopOnce();
            runGameLoopOnce();
            }
        
        shouldDrawNintendoLogo = false;
        nintendoLogoSpriteID = -1;
        indiePubLogoSpriteID = -1;

        // back to blank slate with textures, ready for game to load
        clearAllTextures();
        }
    


    // sound mutex is needed even if sound isn't running.
    OS_InitMutex( &soundMutex );



    
    // init in platform-independent code
    gameInit();
    
    printOut( "Free bytes on heap after init=%d\n",
              OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );




    // for now, no sound process at all on a clone
    if( ! isThisAClone() ) {
        
        // start sound processes
        printOut( "Starting sound process\n" );


        OS_CreateThread( &soundThread, soundThreadProcess, NULL, 
                         soundThreadStack + 
                         SOUND_THREAD_STACK_SIZE / sizeof(unsigned int), 
                         SOUND_THREAD_STACK_SIZE,
                         SOUND_THREAD_PRIORITY );
        OS_WakeupThreadDirect( &soundThread );

    
        u32 channelMask = 0;

        for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {        
            // add a 1 in the mask here
            channelMask = channelMask | ( 0x1 << i );        
            }


        SND_LockChannel( channelMask, 0 );
    
        //int stereoSpread = 127 / MAX_SOUND_CHANNELS;
    

        for( int i=0; i<MAX_SOUND_CHANNELS; i++ ) {

            SND_SetupChannelPcm( i, SND_WAVE_FORMAT_PCM16,
                                 soundBuffer[i],
                                 SND_CHANNEL_LOOP_REPEAT, 0, 
                                 ( SOUND_BUFFER_SIZE * sizeof(s16) ) / 
                                    sizeof(u32),
                                 // volume off by default
                                 0,
                                 SND_CHANNEL_DATASHIFT_NONE, soundTimerValue, 
                                 // panned center by default
                                 64  );
            soundBufferPage[i] = 0;
            memset( soundBuffer[i], 0, SOUND_BUFFER_SIZE * sizeof(s16) );
            }
    
    
        // one alarm callback for all channels
        int alarmNumber = 0;
        SND_SetupAlarm( alarmNumber, soundAlarmPeriod, 
                        soundAlarmPeriod, SoundAlarmCallback, NULL );

        SND_StartTimer( channelMask, 0, 
                        (unsigned int)( 1 << alarmNumber ), 0 );

        SND_FlushCommand( SND_COMMAND_NOBLOCK );
        }
    
    
    while( true ){
        runGameLoopOnce();
        }
    

    OS_Terminate();
    
    }


static char lidIsClosed = false;


char getLidClosed() {
    return lidIsClosed;
    }


static char hasStartBeenPressed = false;


char getPauseButtonPressed() {
    char returnVal = hasStartBeenPressed;
    
    hasStartBeenPressed = false;
    
    return returnVal;
    }



void runGameLoopOnce() {
    G3X_Reset();

    // camera
    VecFx32 cameraPos = { 0, 0, 0 };
    VecFx32 lookAt = { 0, 0, -FX32_ONE };
    VecFx32 upVector = { 0, FX32_ONE, 0 };

    G3_LookAt( &cameraPos, &upVector, &lookAt, NULL );

    G3_PushMtx();

        
    G3_MtxMode( GX_MTXMODE_TEXTURE );
    G3_Identity();
    G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

        
    // reset fake z buffer coordinate
    drawZ = farZ;
    polyID = 0;
        

    if( isEvenFrame ) {
        drawTopScreen();

        if( shouldDrawNintendoLogo ) {
            rgbaColor logoColor = { 255, 255, 255, nintendoLogoFade };
            
            drawSprite( indiePubLogoSpriteID, 64, 64, logoColor );
            }


        //printOut( "Free bytes on heap after drawTop=%d\n",
        //  OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );
            
        // game loop every-other screen
        gameLoopTick();
        // same for network step
        stepNetwork();

            
        if( !isCloneChild ) {
            stepCloneBootParent();

            checkForFileRequest();
            }
        }
    else {
        drawBottomScreen();
            
        if( shouldDrawNintendoLogo ) {
            rgbaColor logoColor = { 255, 255, 255, nintendoLogoFade };

            drawSprite( nintendoLogoSpriteID, 0, 32, logoColor );
            }


        //printOut( "Free bytes on heap after drawBottom=%d\n",
        //         OS_CheckHeap( OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE ) );
        }
    G3_PopMtx( 1 );
        
    // swap buffers
    OSIntrMode oldMode = OS_DisableInterrupts();
    G3_SwapBuffers( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    shouldSwap = true;
    OS_RestoreInterrupts( oldMode );


    if( PAD_Read() & PAD_BUTTON_START ) {

        // ignore start button on menu screens or when already paused
        if( ! isMainMenuShowing() && 
            ! isPaused() ) {
            
            hasStartBeenPressed = true;
            }
        }
    

    BOOL detectFold = PAD_DetectFold();
    
    if( detectFold && ! lidIsClosed ) {
        // turn screen off
        GX_DispOff();
        GXS_DispOff();
        
        PM_SetLCDPower( PM_LCD_POWER_OFF );
        
        lidIsClosed = true;
        }
    else if( !detectFold && lidIsClosed ) {
        // opened back up
        
        // careful:  this call may fail if less than 100ms have passed
        // since closed.
        // If so, try again later (don't reset lidIsClosed)
        
        if( PM_SetLCDPower( PM_LCD_POWER_ON ) ) {
            GX_DispOn();
            GXS_DispOn();
            
            lidIsClosed = false;
            }
        }
    
    
        
    // interrupt will wake swapThread up
    OS_WaitVBlankIntr();


    // receive ARM7 command replies
    while( SND_RecvCommandReply( SND_COMMAND_NOBLOCK ) != NULL ) {
        }
            


    // do one step of texture replacement here
    textureReplacementStep();
    }


void platformSleep( unsigned int inTargetMilliseconds ) {
    OS_Sleep( inTargetMilliseconds );
    }




void setSoundChannelVolume( int inChannelNumber, int inVolume ) {
    SND_SetChannelVolume(
        (unsigned int)( 0x1 << inChannelNumber ),
        inVolume,
        SND_CHANNEL_DATASHIFT_NONE );

    BOOL result = SND_FlushCommand( SND_COMMAND_NOBLOCK );

    if( ! result ) {
        printOut( "SND_FlushCommand failed when trying to set volume\n" );
        }
    }


void setSoundChannelPan( int inChannelNumber, int inPan ) {
    SND_SetChannelPan(
        (unsigned int)( 0x1 << inChannelNumber ),
        inPan );

    SND_FlushCommand( SND_COMMAND_NOBLOCK );
    }


void lockAudio() {
    OS_LockMutex( &soundMutex );    
    }


void unlockAudio() {
    OS_UnlockMutex( &soundMutex );    
    }
