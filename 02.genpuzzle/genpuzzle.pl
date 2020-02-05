#!/usr/bin/perl

use strict;
use warnings;
use v5.10;	# for say etc
use Function::Parameters;
use Getopt::Long;
use Data::Dumper;

#
# my( $b, $w ) = score( $guess, $secret, $n );
#	Given 2 $n-letter strings, $guess and $secret,
#	score the guess against the secret, returning
#	($b,$w) where $b is the number of exact matches,
#	same colour in same position, and $w is the number
#	of approximate matches (unmatched colours matching
#	the same colour in a different position).
#
fun score( $guess, $secret, $n )
{
	my @g = split( //, $guess );
	my @s = split( //, $secret );
	my @gf = (1) x $n;	# gf[i] == is g[i] free for matching?
	my @sf = (1) x $n;	# sf[i] == is s[i] free for matching?
	my $b = my $w = 0;
	foreach my $i (0..$n-1)
	{
		if( $g[$i] eq $s[$i] )
		{
			$b++;
			$gf[$i] = 0;
			$sf[$i] = 0;
		}
	}
	foreach my $i (0..$n-1)
	{
		next unless $gf[$i];
		foreach my $j (0..$n-1)
		{
			next unless $gf[$i] && $sf[$j];
			if( $g[$i] eq $s[$j] )
			{
				$w++;
				$gf[$i] = 0;
				$sf[$j] = 0;
			}
		}
	}
	return ( $b, $w );
}


#
# my @allsecrets = makeallsecrets( $n, $colours );
#	Given $n (the number of holes in a secret - and guess)
#	and $colours, a string containing all the distinct colour
#	letters, build and return an array of all possible n-letter secrets
#	containing combinations of those colours, with or without
#	repititions.
#
fun makeallsecrets( $n, $colours )
{
	my @c = split( //, $colours );
	my @result;
	if( $n > 1 )
	{
		my @s = makeallsecrets( $n-1, $colours );
		foreach my $c (@c)
		{
			push @result, map { $_.$c } @s;
		}
	} else
	{
		@result = @c;
	}
	return @result;
}


#
# my @match = filtersecrets( $n, $guess, $b, $w, @secrets );
#	Given $guess (an $n letter string), $b and $w, it's score against
#	the unknown secret, and @secrets (an array of all possible secrets,
#	each an $n letter string), filter out all @secrets that would not
#	score $guess as ($b,$w).  Returns the shorter list of secrets that
#	match this "score $guess as ($b,$w)" constraint.
#
fun filtersecrets( $n, $guess, $b, $w, @secrets )
{
	return grep {
		my( $b2, $w2 ) = score( $guess, $_, $n );
		$b==$b2 && $w==$w2;
	} @secrets;
}


#
# my $secret = randomsecret( $colours, $nholes );
#	Make a random secret - $nholes long, each letter being
#	a free choice from the letters in $colours.
#
fun randomsecret( $colours, $nholes )
{
	my @c = split( //, $colours );
	my $secret="";
	foreach my $n (1..$nholes)
	{
		my $randpos = int(rand(length($colours)));
		my $ch = substr( $colours, $randpos, 1 );
		$secret .= $ch;
	}
	return $secret;
}


#
# my( $nsol, $guesslist, $scorelist ) = randomsearch( $n, $colours, $secret );
#	Given $n, the number of holes to find, and $colours, a list of
#	all possible colours, and secret $secret,
#	make random matching guesses, scoring each guess against the
#	secret, and filter out all possible solutions that don't match,
#	until there are only 1 or 0 solutions.  Return the number of
#	solutions $nsol (0 or 1) and $guesslist, a reference to the array
#	of guesses that led to the solution (or lack of it), and $scorelist,
#	a ref to the array of (b,w) pairs.
#
fun randomsearch( $n, $colours, $secret )
{
	#say "debug: secret=$secret";
	my @allposs = makeallsecrets( $n, $colours );
	my @guess;
	my @score;
	do {
		my $nposs=@allposs;
		my $pos = int(rand($nposs));
		my $guess = $allposs[$pos];
		push @guess, $guess;
		my( $b, $w ) = score( $guess, $secret, $n );
		push @score, "$b-$w";
		#say "debug: nposs=$nposs, pos=$pos, guess=$guess, b=$b, w=$w";
		@allposs = filtersecrets( $n, $guess, $b, $w, @allposs );
	} while( @allposs > 1 );
	return ( scalar(@allposs), \@guess, \@score );
}

my $shortest = 1000;	# length of shortest list of guesses found
my @sg;			# list of shortest guesses
my @ss;			# list of shortest scores
my $s_secret;		# the corresponding secret

my $quiet = 0;
my $result = GetOptions( "quiet" => \$quiet );

die "Usage: genpuzzle [--quiet] NHOLES COLOURS [NITERATIONS]\n" unless
	$result && (@ARGV==2 || @ARGV==3);

my $nholes = shift;	# eg 4
my $colours = shift;	# eg "rgby"
my $niter = shift // 1000;

die unless length($colours)>=$nholes;

foreach my $it (1..$niter)
{
	my $secret = randomsecret( $colours, $nholes );
	my( $nsol, $guesslist, $scorelist ) =
		randomsearch( $nholes, $colours, $secret );
	next unless $nsol==1;
	my $nguess = @$guesslist;
	next if $guesslist->[$nguess-1] eq $secret;
	say "secret=$secret, nsol=$nsol, guesses=", join(',',@$guesslist)
		unless $quiet;
	if( $nguess < $shortest )
	{
		$shortest = $nguess;
		@sg = @$guesslist;
		@ss = @$scorelist;
		$s_secret = $secret;
	}
}
say "holes=$nholes, colours=$colours: shortest=$shortest, secret=$s_secret, shortest guesses=", join(',',@sg), ", shortest scores=", join(',', @ss) if $s_secret;
