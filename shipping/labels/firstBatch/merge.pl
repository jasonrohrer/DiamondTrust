#!/usr/bin/perl -w 


my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 3 ) {
    usage();
    }

open( CSV_FILE, $ARGV[0] ) or usage();



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

@columnNames  = split( /,/ );

foreach $name (@columnNames) {
	$name =~ s/\"//g;
	print "column $name\n";
}



while( <CSV_FILE> ) {
    chomp;
	@lineFields  = split( /,/ );

	foreach $field (@lineFields) {
		$field =~ s/\"//g;
	}

	$filledTemplate = $template;

	
	
	$index = 0;
	foreach $name (@columnNames) {
		$fieldValue = $lineFields[$index];

		if( ( $filledTemplate =~ m/\\insert$name/ ) ) {
			print "Template contains \\insert$name, replacing with $fieldValue\n";
			$filledTemplate =~ s/\\insert$name/$fieldValue/g;
		}
		$index ++;
	}


	print OUTPUT $filledTemplate;

	



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
