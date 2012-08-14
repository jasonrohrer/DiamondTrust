#!/usr/bin/perl -w 


use Text::CSV_PP;

my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 4 ) {
    usage();
    }

open( CSV_FILE, $ARGV[0] ) or usage();


my $csvParser = Text::CSV_PP->new();

$csvParser->binary(1);


open( TEMPLATE, $ARGV[1] ) or usage();

$template = "";
while( <TEMPLATE> ) {
	$template .= $_;
}
close TEMPLATE;


open( TEMPLATE, $ARGV[2] ) or usage();

$templateUSA = "";
while( <TEMPLATE> ) {
	$templateUSA .= $_;
}
close TEMPLATE;



open( OUTPUT, ">$ARGV[3]" ) or usage();



print "opening $ARGV[0] for reading\n";

# read header for column names
$_ = <CSV_FILE>;


my @columnNames = ();

print "Trying to parse line $_\n";
if( $csvParser->parse($_) ) {
	@columnNames = $csvParser->fields();
}


foreach $name (@columnNames) {
	print "column $name\n";
}


$countryIndex = -1;
$index = 0;
foreach $name (@columnNames) {
	if( $name eq "Country" ) {
		$countryIndex = $index;
	}
	$index ++;
}


while( <CSV_FILE> ) {
    if( $csvParser->parse($_) ) {
		@lineFields = $csvParser->fields();		
		
		# check if this is a USA address
		$usa = 0;

		if( $countryIndex != -1 ) {
			$fieldValue = $lineFields[$countryIndex];
				
			if( $fieldValue eq "USA" ) {
				$usa = 1;
			}
		}

		$filledTemplate = $template;
		
		if( $usa ) {
			$filledTemplate = $templateUSA;
		}

			
	
		$index = 0;
		foreach $name (@columnNames) {
			$fieldValue = $lineFields[$index];

			if( ( $filledTemplate =~ m/\\insert$name/ ) ) {
				$fieldValue =~ s/\#/\\\#/g;
				
				if( $name eq "ShipCode" ) {
					$fieldValue =~ s/;/\\\\/g;
				}
				$filledTemplate =~ s/\\insert$name/$fieldValue/g;
			}
			elsif( ( $filledTemplate =~ m/\\insertNewline$name/ ) ) {
				$fieldValue =~ s/\#/\\\#/g;
				if( $fieldValue ne "" ) {
					$filledTemplate =~ 
						s/\\insertNewline$name/$fieldValue\\\\/g;
				}
				else {
					$filledTemplate =~ s/\\insertNewline$name//g;
				}
			}
			$index ++;
		}


		print OUTPUT $filledTemplate;

	}

}
close OUTPUT;


sub usage {
    print "Usage:\n";
    print "  merge.pl csv_file template_INTL_file template_USA_file output_file\n";
    print "Example:\n";
    print "  merge.pl test.csv testIn.tex testUSA.tex testOut.tex\n";
    die;
    }
