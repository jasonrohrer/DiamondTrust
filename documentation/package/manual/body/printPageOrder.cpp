#include <stdio.h>
#include <stdlib.h>

int main( int inNumArgs, const char **inArgs ) {
    
    if( inNumArgs != 3 ) {
        printf( "Usage:\n" );
        printf( "  (blank_fill_page used to fill out "
                "to a multiple of 4 pages)\n" );
        printf( "  printPageOrder num_pages blank_fill_page\n" );
        exit(1);
        }
    
    int numPages;
    sscanf( inArgs[1], "%d", &numPages );
    
    int blankFillPage;
    sscanf( inArgs[2], "%d", &blankFillPage );



    int numBlank = 0;
    
    while( numPages % 4 != 0 ) {
        numBlank ++;
        numPages ++;
        }

    int halfPages = numPages / 2;
    
    char front=false;
    
    for( int i=1; i<=halfPages; i++ ) {

        int beginPage = i;
        int endPage = numPages - i + 1;
        

        if( numBlank > 0 ) {
            endPage = blankFillPage;
            numBlank --;
            }
        
        if( front ) {
            // front of a sheet
   
            printf( "%d,%d", i, endPage  );
            }
        else {
            // back of a sheet
            
            printf( "%d,%d", endPage, i );
            }
   
        if( i != halfPages ) {
            printf( "," );
            }

        front = !front;
        }

    printf( "\n" );

    return 0;    
    }
