/*
 * Modification History
 *
 * 2004-October-7    Jason Rohrer
 * Created.
 *
 * 2006-February-19    Jason Rohrer
 * Fixed an inconsistency in memory management.
 *
 * 2008-September-17    Jason Rohrer
 * Support for setting language data directly.
 *
 * 2008-September-17    Jason Rohrer
 * Customized for Diamonds project (so it doesn't touch files).
 */

#include "TranslationManager.h"

#include "minorGems/util/stringUtils.h"



// will be destroyed automatically at program termination
TranslationManagerStaticMembers TranslationManager::mStaticMembers;




void TranslationManager::setLanguageData( const char *inData ) {
    mStaticMembers.setTranslationData( inData );
    }



const char *TranslationManager::translate( const char *inTranslationKey ) {

    char *translatedString = NULL;

    SimpleVector<char *> *keys =
        mStaticMembers.mTranslationKeys;

    SimpleVector<char *> *naturalLanguageStrings =
        mStaticMembers.mNaturalLanguageStrings;

    
    if( keys != NULL ) {
        int numKeys = keys->size();

        for( int i=0; i<numKeys && translatedString == NULL; i++ ) {

            if( strcmp( inTranslationKey, *( keys->getElement( i ) ) ) == 0 ) {
                // keys match
                translatedString =
                    *( naturalLanguageStrings->getElement( i ) );
                }
            }
        }

    
    if( translatedString == NULL ) {
        // no translation exists

        // the translation for this key is the key itself

        // add it to our translation table

        char *key = stringDuplicate( inTranslationKey );
        char *value = stringDuplicate( inTranslationKey );

        keys->push_back( key );
        naturalLanguageStrings->push_back( value );

        // thus, we return a value from our table, just as if a translation
        // had existed for this string
        translatedString = value;
        }

    return translatedString;
    }


        
TranslationManagerStaticMembers::TranslationManagerStaticMembers()
        : mTranslationKeys( NULL ),
          mNaturalLanguageStrings( NULL ) {

    // default
    }



TranslationManagerStaticMembers::~TranslationManagerStaticMembers() {

    if( mTranslationKeys != NULL ) {
        int numKeys = mTranslationKeys->size();

        for( int i=0; i<numKeys; i++ ) {
            delete [] *( mTranslationKeys->getElement( i ) );
            }
        delete mTranslationKeys;
        }

    if( mNaturalLanguageStrings != NULL ) {
        int numKeys = mNaturalLanguageStrings->size();

        for( int i=0; i<numKeys; i++ ) {
            delete [] *( mNaturalLanguageStrings->getElement( i ) );
            }
        delete mNaturalLanguageStrings;
        }

    }


        


static inline const char *stringSkip( const char *inString, 
                                      unsigned int inNumChars ) {
    return &( inString[ inNumChars ] );
    }




void TranslationManagerStaticMembers::setTranslationData( const char *inData ) {
    
    // clear the old translation table
    if( mTranslationKeys != NULL ) {
        int numKeys = mTranslationKeys->size();

        for( int i=0; i<numKeys; i++ ) {
            delete [] *( mTranslationKeys->getElement( i ) );
            }
        delete mTranslationKeys;
        }

    if( mNaturalLanguageStrings != NULL ) {
        int numKeys = mNaturalLanguageStrings->size();

        for( int i=0; i<numKeys; i++ ) {
            delete [] *( mNaturalLanguageStrings->getElement( i ) );
            }
        delete mNaturalLanguageStrings;
        }

    
    // now read in the translation table
    mTranslationKeys = new SimpleVector<char *>();
    mNaturalLanguageStrings = new SimpleVector<char *>();
            

    
    char readError = false;
                
    while( ! readError ) {

        char *key = new char[ 100 ];

        int numRead = sscanf( inData, "%99s", key );

        if( numRead == 1 ) {
            
            inData = stringSkip( inData, strlen( key ) );
            
            
            // skip the first "
            int readChar = ' ';

            while( readChar != '"' && readChar != '\0' ) {
                readChar = inData[0];
                inData = stringSkip( inData, 1 );
                }
            if( readChar != EOF ) {
                char *naturalLanguageString = new char[1000];
                // scan a string of up to 999 characters, stopping
                // at the first " character
                numRead = sscanf( inData, "%999[^\"]",
                                  naturalLanguageString );

                if( numRead == 1 ) {
                    inData = stringSkip( inData, 
                                         strlen( naturalLanguageString ) );

                    // trim the key and string and save them
                    mTranslationKeys->push_back(
                        stringDuplicate( key ) );
                    mNaturalLanguageStrings->push_back(
                        stringDuplicate( naturalLanguageString ) );
                    }
                else {
                    readError = true;
                    }
                delete [] naturalLanguageString;
                
                // skip the trailing "
                readChar = ' ';
                
                while( readChar != '"' && readChar != '\0' ) {
                    readChar = inData[0];
                    inData = stringSkip( inData, 1 );                
                    }
                }
            else {
                readError = true;
                }
            
            }
        else {
            readError = true;
            }
        delete [] key;
        
        }
    }
