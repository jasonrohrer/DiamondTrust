#ifdef SDK_TWL
    #include <twl.h>
#else
    #include <nitro.h>
#endif


#define true 1




#ifdef SDK_TWL
    void TwlMain( void )    
#else
    void NitroMain( void )
#endif
{
    OS_Init();
    OS_Printf( "Hello, World\n" );
    
    while( true ){
        }
    }
