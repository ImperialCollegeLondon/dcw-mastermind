#!/usr/bin/perl
#
#	makeps: 	Reads the output of the reverse mastermind solver,
#			specifically the final line, of the form
#	"holes=4, colours=rgby: shortest=2, secret=bgrb, shortest guesses=grbb,rbgb, shortest scores=1-3,1-3"
#			and writes a corresponding inc file ready to
#			generate Postscript for that precise case..


use strict;
use warnings;
use feature "say";
use Function::Parameters;

die "Usage: makeps basename\n" unless @ARGV==1;
my $basename = shift;

my $incfile = "$basename.inc";
my $psfile = "$basename.ps";

open( my $outfh, '>', $incfile ) || die "makeps: can't create $incfile\n";

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
my $gbwtxt = join( "\n", @gbw );

print $outfh qq(%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

% Shhhhhhhhh! The secret code is $secret

/maxguesses $mg  def
/numholes   $nholes  def


$gbwtxt


%include "mmsupport.i"


%%Page: 1 1
%%PageOrientation: Portrait

/Helvetica-Bold findfont 13 scalefont setfont

/texth fontheight def

maxguesses shownumbers

[ $gstr (     ) ]    showguesses

[ $bwstr ]    showscores

showpage
);

close( $outfh );

system( "./inc $incfile $psfile" );
system( "gv $psfile" );
