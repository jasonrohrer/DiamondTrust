#!/usr/bin/perl -w 


use Text::CSV_PP;

my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 3 ) {
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



open( OUTPUT, ">$ARGV[2]" ) or usage();



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


while( <CSV_FILE> ) {
    if( $csvParser->parse($_) ) {
		@lineFields = $csvParser->fields();
    
		$filledTemplate = $template;

	
	
		$index = 0;
		foreach $name (@columnNames) {
			$fieldValue = $lineFields[$index];

			if( ( $filledTemplate =~ m/\\insert$name/ ) ) {
				$fieldValue =~ s/\#/\\\#/g;
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



    #(my $email, my $name) = split( /,\W*/ );
    #$name =~ s/ /\+/g;

    #print "  email = ($email), name = ($name)\n";

    #my $url = "http://sleepisdeath.net/ticketServer/server.php?action=sell_ticket&security_data=$ARGV[2]&email=$email&name=$name&reference=manual&tags=$ARGV[1]&security_hash=$ARGV[3]";
    #print "  url = $url\n";
    #$result = `$wgetPath "$url" -q -O -`;

    #print "  result = $result\n";
    }
close OUTPUT;


sub usage {
    print "Usage:\n";
    print "  merge.pl csv_file template_file output_file\n";
    print "Example:\n";
    print "  merge.pl test.csv testIn.tex testOut.tex\n";
    die;
    }
