#!/usr/bin/perl
#
# ---------------------------------------------------------------------
# Written by: Ben (sk) Sgro, ProjectSkyline[dot]com, 2007.

use lib "$ENV{HOME}/.cpan/build/Authen-SASL-2.10/lib/";
use HTML::LinkExtor; # Extract links from HTML documents.
use LWP::UserAgent;  # For fake user agent
use Tie::File;       # Treat file as array.
use Net::SMTP_auth;  # For SMTP_auth->( )
use HTML::Entities;  # For decode_entities( )
use Time::HiRes      qw(gettimeofday);
use Fcntl;           # For sysopen( )

our $constVersion  = 'alpha - 31.Oct.07';
our $constSoftware = "poser $constVersion by Ben Sgro";
our $constUA = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.0.9)"
             . "Gecko/20061206 Firefox/1.5.0.9";

# file we write||read data to/from
our $constLinksFile    = 'poser_links.txt';   # Links we've visited
our $constMailTemplate = 'mail_template.txt'; # Email to send to matches
our $constConfigFile   = 'poser.conf';        # Configuration for SMTP..
our $constLookupFile   = 'poser_lookup.txt';  # store the subject:url here

# Our 'reply criteria' -- A posting must contain this anywhere inside it.
# We are expanding this to include a weight for each word.
#
our @RC = ( 
            # services we offer
            ['web design',                 1],
            ['custom css',                 1],
            ['graphic design',             1],    
            ['need a logo',                1],
            ['need hosting',               1],
            ['web hosting',                1],
            ['seo',                        1],
            ['remote work',                1],
            ['website',                    1],
            ['web site',                   1],
            ['network security',           1],
            ['application security',       1],
            ['search engine optimization', 1],
            ['internet marketing',         1],
            ['content management system',  1],
            ['penetration testing',        1],
            ['security audit',             1],
            ['custom scripts',             1],
            ['software installation',      1],
            ['802.11',                     1],
            # tools we use
            ['php',                        2],
            ['perl',                       2],
            ['adobe photoshop',            2],
            ['adobe flash',                2],
            ['flash animation',            2],
            ['adobe illustrator',          2],
            ['abobe',                      2],
            ['indesign',                   2],
            ['sql',                        2],
            ['flash banner',               2],
            ['identity',                   2],
            ['mysql',                      2],
            ['sql',                        1],
            ['cms',                        2],
            ['ajax',                       2],
            ['dreamweaver',                2], 
            ['smarty',                     2],
            # tools we dont use
            ['cold fusion',               -4],
            ['asp',                       -4],
            ['.net',                      -4],
            ['java',                      -4],
            ['joomla',                    -4],
            ['plone',                     -4],
            ['drupal',                    -4],
            ['ruby',                      -4],
            ['phpnuke',                   -4],
            ['voip',                      -4],
            # words that could mean we are off target
            ['drafter',                   -4],
            ['production artist',         -4],
            ['stylist',                   -4],
            ['make-up',                   -4],
            ['modeling',                  -4],
            ['models',                    -4],
            ['p/t',                       -4],
            ['hostes',                    -4],
            ['ebay',                      -4],
            ['volunteer',                 -4],
            ['ladies',                    -4],
            ['focus group',               -4],
            ['sales rep',                 -4],
            ['data entry',                -4],
            ['data-entry',                -4],
            ['staffing',                  -4],
            ['female',                    -4], 
            ['singer',                    -4],
            ['short film',                -4],
            ['photographer',              -4],
            ['google maps',               -4],
            ['non-profit',                -4],
            ['non profit',                -4],
            ['event staff',               -4],
            ['massage girl',              -4],
            ['pictures of',               -4],
            ['performance',               -4],
            ['architect',                 -4],
            ['assistant',                 -4],
            ['film video',                -4],
            ['quickbooks',                -4],
            ['google maps',               -4],
            ['senior manager',            -4],
            ['seeking to share',          -4],
            ['hairdresser',               -4],
            ['training',                  -4],
            ['recent college grad',       -4],
            ['manager',                   -4],
            ['earn $',                    -4],
            ['need a new job',            -4],
            ['character animator',        -4],
            ['street team',               -4],
            ['seamstress',                -4],
            ['convert',                   -4],
            ['actors',                    -4],
            ['actresses',                 -4],
            ['take a l',                  -4], 
            ['hiring',                    -4],
            ['office addministrator',     -4],
            ['great opportunity!',        -4],
            ['promotion girls',           -4],
            ['bartender',                 -4],
            ['sew',                       -4],
            ['dj',                        -4],
            ['party',                     -4],
            ['girls',                     -4],
            ['bouncers',                  -4],
            ['at home work',              -4],
            ['dance',                     -4],
            ['free',                      -4],
            ['excellent opportunity',     -4],
            ['video shoot',               -4],
            ['get paid for',              -4],
            ['storyboard',                -4],
            ['i need you to',             -4],
            # dont respond to this
            ['evigilant',                -10],
            ['evigilant.com',            -10],
            ['no companies',             -10],
            ['local candidates',         -10],   
            ['locals only',              -10],
            ['local only',               -10],
            ['full time',                -10],
            ['fulltime',                 -10],
            ['full-time',                -10],
            ['part-time',                -10],
            ['part time',                -10],
            ['hiring locally',           -10],
            ['looking for a college student', -10],
            ['looking to barter',             -10],
            ['in house',                      -10],
            ['intern',                        -10],
            ['on call',                       -10],
            ['want to rent',                  -10],
            ['want to buy',                   -10],
            ['need to sell',                  -10],
           );
our $SIZE_OF_RC = scalar @RC;



# usage( ) - display program usage
sub usage
{
    print $constSoftware . "\n";
    print "Usage: ./poser.pl <URL> <CATEGORY> \n";
    print "example: ./poser http://newyork.craigslist.org /cpg\n";
    exit;
}

# createReadFile( ) - create the LinksFile if it doesn't exist
sub createReadFile
{
    sysopen(readFH, $constLinksFile, O_CREAT); 
    close(readFH);
}

# createReadFile( ) - create the poser_lookup.txt file if it doesn't exist.
sub createLookupFile
{
    sysopen(readFH, $constLookupFile, O_CREAT);
    close(readFH);
}


# $theURL = URLDecode($theURL)
# http://glennf.com/writing/hexadecimal.url.encoding.html
sub URLDecode 
{
    my $theURL = $_[0];
    $theURL =~ tr/+/ /;
    $theURL =~ s/%([a-fA-F0-9]{2,2})/chr(hex($1))/eg;
    $theURL =~ s/<!--(.|\n)*-->//g;
    return $theURL;
}

# sendEmail($serv, $user, $pass, $mailTo, $subject) - sends email to specified 
# address.
sub sendEmail
{
    my ($servStr) = shift (@_);    
    my ($userStr) = shift (@_);
    my ($passStr) = shift (@_);
    my ($mailTo)  = shift (@_);
    my ($subject) = shift (@_);

    ###################
    # code for Gmail. #
    ###################
    # If you want to use this, uncomment it.
    #
    #$messageBody = '';
    #my $gmail = Mail::Webmail::Gmail->new(
	#username => $userStr, 
	#password => $passStr,
	#); 

    #$r1 = $mailTo;
    #my $mailSet = [$r1, $r2,]; # Array of recipients.

    # Load the mail_template message
    #tie @mailData, Tie::File, $constMailTemplate;
    #foreach $mailStr (@mailData)
    #{
	#$messageBody .= $mailStr . "\n";
    #}
    #untie @mailData;

    # Send the mail
    #$gmail->send_message( to      => $r1,
    #			  subject => $subject,
    #			  msgbody => $messageBody 
    #	                );


    $smtp = Net::SMTP_auth->new($serv, Port => 80);
    $rAuth = $smtp->auth("LOGIN", $userStr, $passStr); 
    if ( $rAuth == ''  )
    {
        print "Authentication failed, check AUTH type, username & password\n";
        $smtp->quit( ); # Connection is still open.
        exit;
    }

    $smtp->mail($userStr);

    $r1 = $mailTo;
    $r2 = '' #'your@email.com';
    $smtp->recipient($r1, $r2);
    
    $smtp->data( );
    $smtp->datasend('To: ' . $r1);
    $smtp->datasend("\n");
    $smtp->datasend('CC: your@email.com');
    $smtp->datasend("\n");
    $smtp->datasend('From: your@email.com');
    $smtp->datasend("\n");
    $smtp->datasend("Subject: $subject\n");
    $smtp->datasend("\n");

    # Load the email from file
    tie @mailData, Tie::File, $constMailTemplate;
    foreach $mailStr (@mailData)
    {
        $smtp->datasend($mailStr . "\n");
    }
    untie @mailData;

    $smtp->datasend("\n");
    $smtp->dataend( );
    $smtp->quit( );
}

# replyCriteria( ) - Does the argument meet the reply true criteria?
sub replyCriteria
{
    my ($userStr) = shift (@_);
    my ($passStr) = shift (@_);
    my ($servStr) = shift (@_);
    my ($htmlStr) = shift (@_);
    my ($linkStr) = shift (@_);

    @tmpRC   = @RC;
    @htmlSet = split("\n", $htmlStr);
    @pageSet = split("\n", $htmlStr); # to find the email

    # Jun-27; Changed to see effects.
    $weight     = -3; # default is -3
    $rcDebugStr = ''; # store the strings we 'meet' here
    
    # Loop through each line of the HTML page.
    foreach $lineStr (@htmlSet)
    {
        # Here we have all our 'reply criteria'. If enough of these
        # are met, we send an email to this poster.
        for ($count = 0; $count < $SIZE_OF_RC; $count++)
        {
            $rcStr  = $tmpRC[$count][0];
            $rcVal  = $tmpRC[$count][1];
            # Does the current line contain any of our criteria?
            if ( $lineStr =~ m/$rcStr/i )
            {
                # Lets calculate the weight.
                $weight = $weight + ($rcVal);

                # Once we find a match, we do not want to calculate
                # its value in weight again. Set the value to zero
                # in $tmpRC.
                $tmpRC[$count][1] = 0;

                # Save the match, so we can dislay it in debug
                $rcDebugStr .= '[' . $rcStr . ']';
            }
        }
    }
    if ( $weight > 0 )
    {
        # This is a post we want to email.
        # We've found criteria, now lets find the email address
        # and subject of this posting.
        foreach $pageStr (@pageSet)
        {
            if ( $pageStr =~ m/Reply to: <a href=/ )
            {
                # We've found the email string w/attached
                # subject. However, its Numeric Character Encoded.
                # http://www.i18nguy.com/markup/ncrs.html
                # - thanks galt/innercircle.
                decode_entities($pageStr);
                
                # Fetch the email address & subject
                # - thanks pack/innercircle.
                my ($mailTo,$subject) = $pageStr =~
                    /.*$mailTo\:(.*)\?subject=(.*)\"/;
                $subject = URLDecode($subject); #decode subject ..%20..
                
                # Send the Email
                sendEmail($serv, $user, $pass, $mailTo, $subject);

                # We've sent a mail to this user, lets record
                # the subject:url in case we want to look it
                # up later.
                # This has been expanded to save the weight, and
                # the reply criteria keywords we've found/met.
                tie @lookupData, Tie::File, $constLookupFile;
                $lookupNum = @lookupData;
                $lookupData[$lookupNum] = $subject . ':' . $linkStr
                                        . ' rc met ' . $rcDebugStr
                                        . ' weight(' . $weight . ')';
                untie @lookupData;                

                return 0;
            }
        }
    }
    quick_exit:
    return -1;
}


#
# MAIN 
#

$t0 = gettimeofday( ); # Start the time

if ( $#ARGV == '' ) { usage( ); } # Check arguments

do "poser.conf"; # Read configuration data from poser.conf

createReadFile( );   # create the file if it does not exist
createLookupFile( ); # ditto ^

$URL    = shift @ARGV; # base URL
$dir    = shift @ARGV; # CL chategory

$cgURL  = $URL . $dir;

# Create our user agent
my $userAgent = LWP::UserAgent->new( );
$userAgent->agent($constUA);
#if ( $proxy != '' )
#{
#    printf "here";
#    $userAgent->proxy('http', $proxy);
#}

# Create our referer and fetch $URL
my $response = $userAgent->get($cgURL, Referer => "http://www.google.com");

if ( $response->is_error( ))
{
    printf "%s\n", $response->status_line;
    exit;
}

# Fetch the page
$content = $response->content( );

# Lets extract all the URLS from this page
$parser  = HTML::LinkExtor->new( );
$parser->parse($content);
@links = $parser->links; #returns all links

# Loop through each link
$mailCount      = 0;
$lookupCount    = 0;
$duplicateCount = 0;
$noMatch        = 0;
foreach $linkSet (@links)
{
    my @element  = @$linkSet;
    my $elt_type = shift @element;
    while (@element)
    {
        my ($attrName, $attrValue) = splice(@element, 0, 2);
        # We only care about specific URLs, ones that match $URL
        # or more specifically, $cgURL...

        #load in the entire file of links we've visited
        tie @fileData, Tie::File, $constLinksFile;

        $testStr = "http://add.my.yahoo.com/rss?url=".$cgURL."/index.rss";
        if ( ($attrValue !~ m/index.rss/) && ($attrValue =~ m/$URL/i) )
        { 
            $lookupCount++;
            # Valid links for the page, now we need to check for
            # 1) did we already check/email this link?
            # 2) if not, does it meet our 'reply to' criteria?

            $duplicate  = 0; # Off by default.
            if ( @fileData != '' )
            {
                # Check every line in the file for the current link.
                $recordNum  = @fileData;
                foreach $visitedStr (reverse(@fileData))
                {
                    if ( $visitedStr eq $attrValue )
                    {
                        $duplicate = 1;
                    }
                }
            }                

            if ( $duplicate == 0 ) # We DID NOT see this string in the file.
            {
                # Does it meet the criteria we are searching for?
                my $page = $userAgent->get($attrValue,
                                           Referer => "http://www.google.com");
                # Fetch the page
                $pageStr = $page->content( );
                
                $status = &replyCriteria($user, $pass, $serv, $pageStr,
                                         $attrValue);
                if ( $status == -1 )
                {
                    # This post doesn't match any criteria is weighted <= 0
                    $noMatch++;
                }
                else
                {
                    $mailCount++;                   
                }

                # Add this link to the list so we dont SPAM it.
                $recordNum = @fileData;
                $fileData[$recordNum] = $attrValue;
            }
            else
            {
                # We've already looked up this link/post.
                $duplicateCount++;

		# If we see 5 duplicates, we are not going to 
		# see any new ones ... skip this and continue to
		# the next city.
		if ( $duplicateCount >= 5 )
		{
		    goto display;
		}
            }
            untie @fileData;
        }
    }  
}

display:
# Display output for the user

$time     = gettimeofday( ) - $t0;
$proxyStr = ( $proxy == '' )? 'disabled' : " $proxy enabled";

print "Parent URL: $cgURL\n";
printf "Completed $lookupCount attempted lookups in %.2f seconds.\n", $time;
print "$mailCount posts responded to, $duplicateCount duplicates found.\n";
print "$noMatch posts found that did not match 'reply criteria'.\n";
print "Proxy $proxyStr.\n";
print "---------------------------------------------------------\n";
# EOF
