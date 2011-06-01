#!/usr/bin/perl -w


my $numArgs = $#ARGV + 1;

if( $numArgs < 2 ) {
	print "Usage:\n";
	print "  compileFilesIn.pl outFile.cpp dataFile1 dataFile2 ... " .
		"dataFileN\n\n";
	exit 1;
}

	
$outFileName = $ARGV[0]; 


$numFiles = $#ARGV;


open( OUTFILE, ">$outFileName" ) or die;

@dataArrayNames = ();
@dataArrayLengths = ();


print OUTFILE "const int numIncludedDataFiles = $numFiles;\n\n";


print OUTFILE "const char *includedDataFileNames[$numFiles] = {\n";

for( $i=0; $i<$numFiles; $i++ ) {
	
	$fileName = $ARGV[ 1 + $i ];
	
	# remove path
	$fileName =~ s{.*/}{};

	print OUTFILE "\"$fileName\"";

	if( $i < $numFiles - 1 ) {
		print OUTFILE ",";
	}
	print OUTFILE "\n";

	push( @dataArrayNames, "includedData$i" );
}
print OUTFILE "};\n\n\n";







print OUTFILE "// one array per file, with all data as hex\n\n";

for( $i=0; $i<$numFiles; $i++ ) {

	$fileName = $ARGV[ 1 + $i ];

	# remove path
	$fileNameShort = $fileName;
	$fileNameShort =~ s{.*/}{};

	$fileData = readBinaryFile( $fileName );

	$fileLength = length( $fileData );

	push( @dataArrayLengths, $fileLength );


	$dataArrayName = $dataArrayNames[$i];

	print OUTFILE "// for data file $fileNameShort :\n";
	print OUTFILE "const unsigned char $dataArrayName\[$fileLength\] = {\n";


	$byteNumber = 0;
	foreach( split( //, $fileData ) ) {
		printf( OUTFILE "0x%02x", ord($_) );
		$byteNumber++;

		if( $byteNumber < $fileLength ) {
			print OUTFILE ", ";
		}
		if( $byteNumber % 8 == 0 ) {
			print OUTFILE "\n";
		}
	}


	print OUTFILE "};\n\n\n";
}




print OUTFILE "const unsigned char *includedDataArrays[$numFiles] = {\n";

for( $i=0; $i<$numFiles; $i++ ) {
	
	$dataArrayName = $dataArrayNames[$i];
	
	print OUTFILE "$dataArrayName";

	if( $i < $numFiles - 1 ) {
		print OUTFILE ",";
	}
	print OUTFILE "\n";
}
print OUTFILE "};\n\n\n";



print OUTFILE "const int includedDataArrayLengths[$numFiles] = {\n";

for( $i=0; $i<$numFiles; $i++ ) {
	
	$dataArrayLength = $dataArrayLengths[$i];
	
	print OUTFILE "$dataArrayLength";

	if( $i < $numFiles - 1 ) {
		print OUTFILE ",";
	}
	print OUTFILE "\n";
}
print OUTFILE "};\n\n\n";



# print some inlined C code to define the readIncludedFile utility function
print OUTFILE <<END_OF_FUNCTION;

static unsigned char *readIncludedFile( char *inFileName, int *outSize ) {
    for( int f=0; f<numIncludedDataFiles; f++ ) {
        
        if( strcmp( includedDataFileNames[f], inFileName ) == 0 ) {
            // hit
            int length = includedDataArrayLengths[ f ];
            unsigned char *data = new unsigned char[ length ];
            memcpy( data, includedDataArrays[ f ], (unsigned long)length );
            
            *outSize = length;
            return data;
            }
        }
    // no match
    return NULL;    
    }


END_OF_FUNCTION





##
# Reads file as a string.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFile( "myFile.txt" );
##
sub readFile {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) or die;
    flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}


##
# Reads file as binary.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFile( "myFile.txt" );
##
sub readBinaryFile {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) or die;
    binmode( FILE );
	flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}
