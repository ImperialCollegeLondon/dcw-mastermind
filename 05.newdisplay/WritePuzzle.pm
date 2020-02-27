# WritePuzzle module

use strict;
use warnings;
use Function::Parameters;


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


1;
