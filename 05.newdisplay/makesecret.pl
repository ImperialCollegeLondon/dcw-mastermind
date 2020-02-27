#!/usr/bin/perl
#
#	makesecret: 	Given a secret code on the command line,
#			generates an Encapsulated Postscript image
#			showing that secret code.  Good to produce
#			an overlay to be blutacked onto a Felix printout
#			once you've solved a Mastermind puzzle:-)


use strict;
use warnings;
use feature "say";
use Function::Parameters;

use lib '.';
use WritePuzzle;

die "Usage: makesecret secretcode outputbasename\n" unless @ARGV==2;
my( $secret, $outbasename ) = @ARGV;

my $sepsfile = makesecret( $secret, $outbasename.'s' );
system( "gv -geom +600+10 $sepsfile &" );

say "generated $sepsfile";
exit 0;


#
# my $epsfile = makesecret( $secret, $basename );
#	Write out the $basename.inc postscript-with-includes file
#	that will show the $secret code, then convert it to EPS
#	and return the filename of the EPS file.
#
fun makesecret( $secret, $basename )
{
	my $nholes = length($secret);
	my $defns =
qq(
/maxguesses 1  def
/numholes   $nholes  def

/secret ($secret) def
);

	my $calls =
qq(
[ secret ]    showguesses
);

	my $epsfile = write_inc_file( $basename, $defns, $calls );

	return $epsfile;
}
