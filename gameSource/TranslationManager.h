/*
 * Modification History
 *
 * 2004-October-7    Jason Rohrer
 * Created.
 * Copied structure from SettingsManager.
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

#include "minorGems/common.h"



#ifndef TRANSLATION_MANAGER_INCLUDED
#define TRANSLATION_MANAGER_INCLUDED



#include "minorGems/util/SimpleVector.h"



// utility class for dealing with static member dealocation
class TranslationManagerStaticMembers;



/**
 * Class that manages natural language translation of user interface strings.
 *
 * @author Jason Rohrer
 */
class TranslationManager {



    public:

        // Sets the language data directly without reading it from 
        // the file system.
        // Data string formated the same as a language file.
        // Data string destroyed by caller
        static void setLanguageData( char *inData );
        

        
        
        /**
         * Gets the natural language translation of a key.
         *
         * NOTE:  if a translation does not exist for the key, the key
         *   itself will be returned.  (A copy of the key is returned, so
         *   the original key passed in to translate can be destroyed by caller
         *   if needed).
         *
         * @param inTranslationKey the translation key string.
         *   Must be destroyed by caller if non-const.
         *
         * @return the translated natural language string.
         *   The string MUST NOT be destroyed by the caller, as it will
         *   be destroyed by this class upon program termination.
         *
         * This specification allows the translate function to be used
         * inline, whether or not a correct translation exists.  Thus <PRE>
         *
         * printf( "%s", translate( myKey ) );
         * delete [] myKey;
         * </PRE>
         *
         * will always be correct, whether or not a translation exists, as
         * will <PRE>
         *
         * printf( "%s", translate( "MY_KEY" ) );
         * </PRE>
         */
        static const char *translate( char *inTranslationKey );

        
        
    protected:


        
        static TranslationManagerStaticMembers mStaticMembers;

        
    };



/**
 * Container for static members to allow for their proper destruction
 * on program termination.
 *
 * @author Jason Rohrer
 */
class TranslationManagerStaticMembers {


        
    public:


        
        TranslationManagerStaticMembers();
        ~TranslationManagerStaticMembers();


        // sets the data from a string
        // string contains same contents as a language file
        // string destroyed by caller
        void setTranslationData( char *inData );

        
        
        
        // vectors mapping keys to strings
        SimpleVector<char *> *mTranslationKeys;
        SimpleVector<char *> *mNaturalLanguageStrings;


    };



#endif
