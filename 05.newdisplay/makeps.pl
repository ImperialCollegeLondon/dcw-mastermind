#!/usr/bin/perl
#
#	makeps: 	Reads the output of the 02.genpuzzle/genpuzzle program
#			specifically the final line, of the form
#	"holes=4, colours=rgby: shortest=2, secret=bgrb, shortest guesses=grbb,rbgb, shortest scores=1-3,1-3"
#			and generates an Encapsulated Postscript image
#			for that precise case (via several intermediate forms)..
#			and, now, a second Encapsulated Postscript image
#			showing the secret code for the same case.


use strict;
use warnings;
use feature "say";
use Function::Parameters;

die "Usage: makeps basename\n" unless @ARGV==1;
my $basename = shift;

my( $nholes, $colours, $minn, $secret, $g, $bw );

while( <STDIN> )
{
	chomp;
	next unless /^holes=(\d+), colours=(\w+): shortest=(\d+), secret=(\w+), shortest guesses=(\S+), shortest scores=(\S+)$/;
	( $nholes, $colours, $minn, $secret, $g, $bw ) =
		( $1, uc($2), $3, uc($4), $5, $6 );
}

my $ncolours = length($colours);

die "makeps: ncolours $ncolours must be > 1\n" unless $ncolours > 1;

die "makeps: nholes $nholes must be > 1\n" unless $nholes > 1;

say "nholes=$nholes, colours=$colours, ncolours=$ncolours, minn=$minn, secret=$secret, g=$g, bw=$bw";


#
# my $epsfilename = write_inc_file( $basename, $defns, $calls );
#	Write a Postscript-with-includes file with $defns and $calls,
#	then expand the includes and convert it to EPS.  Return the
#	filename of the EPSfile (should be $basename.eps).
#
fun write_inc_file( $basename, $defns, $calls )
{
	my $incfile = "$basename.inc";
	my $psfile = "$basename.ps";
	my $epsfile = "$basename.eps";

	open( my $outfh, '>', $incfile ) ||
		die "makeps: can't create $incfile\n";

	print $outfh
qq(%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

$defns

%include "mmsupport.i"

%%Page: 1 1
%%PageOrientation: Portrait

/Helvetica-Bold findfont 13 scalefont setfont

/texth fontheight def

$calls

showpage
);

	close( $outfh );

	system( "./inc $incfile $psfile" );

	unlink( $epsfile ) if -f $epsfile;

	system( "ps2eps $psfile" );

	return $epsfile;
}


#
# my $epsfile = makepuzzle( $basename );
#	Write out the $basename.inc postscript-with-includes file
#	that will show the entire puzzle, then convert it to EPS
#	and return the filename of the EPS file.
#
fun makepuzzle( $basename )
{
	my @g = split( /,/, uc($g) );
	my $ng = @g;

	my @bw = split( /,/, $bw );
	my $nbw = @bw;

	die "makeps: shortest guesses has $minn guesses, not $ng\n"
		unless $ng == $minn;

	die "makeps: shortest guesses has $minn guesses, not $nbw blacks+white scores\n"
		unless $nbw == $minn;

	my $mg = $ng+1;

	my @sc;
	my @gbw;

	foreach my $gno (1..$ng)
	{
		my $g = $g[$gno-1];
		my $bw = $bw[$gno-1];
		$bw =~ /^(\d+)-(\d+)$/;
		my( $b, $w ) = ( $1, $2 );
		push @sc, "b$gno w$gno";
		push @gbw,
qq(/g$gno ($g) def
/b$gno $b def
/w$gno $w def

);
	}

	my $gstr = join( " ", map { "g$_" } 1..$ng );
	my $bwstr = join( " ", @sc );
	my $gbwstr = join( "\n", @gbw );

	my $defns =
qq(
% Shhhhhhhhh! The secret code is $secret

/maxguesses $mg  def
/numholes   $nholes  def

$gbwstr
);

	my $calls =
qq(
maxguesses shownumbers

[ $gstr (     ) ]    showguesses

[ $bwstr ]    showscores
);

	my $epsfile = write_inc_file( $basename, $defns, $calls );

	return $epsfile;
}


#
# my $epsfile = makesecret( $basename );
#	Write out the $basename.inc postscript-with-includes file
#	that will show the secret code, then convert it to EPS
#	and return the filename of the EPS file.
#
fun makesecret( $basename )
{
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


my $epsfile = makepuzzle( $basename );
system( "gv $epsfile &" );

my $sepsfile = makesecret( $basename.'s' );
system( "gv -geom +600+10 $sepsfile &" );

say "generated $epsfile and $sepsfile";
