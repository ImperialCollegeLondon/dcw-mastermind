/*
	Mastermind.c:

	Main program for Mastermind.

	Duncan C. White/Peter S. Cutler
	Started 24th July 1992

	C translation, 5th/Dec/2000, Duncan C. White
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <misc.h>
#include <mmsupport.h>


#define	NUMHOLES	3
#define	SPACE		' '
#define	VALIDCOLOURS	"RGBY"


typedef char GuessString[NUMHOLES+1];


// SPECIAL: PARTICULAR DESIRED CODE
void SetupCode( int numholes, char *colours, GuessString code )
{
	code[0] = 'B';
	code[1] = 'B';
	code[2] = 'G';
	code[3] = '\0';
}

void ScoreGuess( int n, GuessString c, GuessString g, int *b, int *w )
{
	int i, j;
	GuessString cc;

	strcpy( cc, c );

	*b = 0;
	for( i = 0; i < n; i++ )
	{
		if( cc[i] == g[i] )
		{
			++*b; g[i] = SPACE; cc[i] = SPACE;
		}
	}

	*w = 0;
	for( i = 0; i < n; i++ )
	{
		for( j = 0; j < n; j++ )
		{
			if( cc[i] == g[j] && g[j] != SPACE )
			{
				++*w; g[j] = SPACE; cc[i] = SPACE;
			}
		}
	}
}


GuessString code, guess;
int b, w;


int main( void )
{
	InitialiseMMSupport( NUMHOLES, VALIDCOLOURS );
	do {
		StartGame();
		SetupCode( NUMHOLES, VALIDCOLOURS, code);
		do {
			GetGuess( guess );
			ScoreGuess( NUMHOLES, code, guess, &b, &w );
			PutScore( b, w );
		} while( b != NUMHOLES && ! BoardFull() );
		FinishGame();
	} while( AnotherGame() );

	CloseDownMMSupport();
	return 1;
}
