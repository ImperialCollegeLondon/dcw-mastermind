/*
	mmsupport.h:

	I/O module for the game of Mastermind.

	Duncan C. White/Peter S. Cutler
	Started 24th July 1992

	C translation, 16/Nov/2000, Duncan C. White
 */


#define maxcolours 26	/* Not more colours than this. */
#define maxholes   10	/* Not more holes than this. */


int Random ( int n );
/*
	Pre:	n >= 1
	Post:	returns a random cardinal in the range 0 to n-1
*/

void InitialiseMMSupport( int numholes, char *validcolours );
/*
	Pre:	numholes <= maxholes;
		Length(validcolours) <= maxcolours.
	Post:	The I/O module has been initialised for Mastermind,
		with numholes     - the number of holes (positions);
		and  validcolours - the set of valid colour characters.
 */

void StartGame( void );
/*
	Pre:	I/O has been initialised.
	Post: 	The screen is set up for a new game.
 */

void GetGuess( char * guess );
/*
	Pre:	Storage has been allocated for guess.
	Post:	A valid guess has been entered by the user,
		stored in guess,
		and displayed on screen.
 */

void PutScore( int blacks, int whites );
/*
	Pre:	blacks + whites <= numholes.
	Post:	The given score is displayed on screen.
 */

void PutGuess( char * guess );
/*
	Pre:	guess is a valid guess.
	Post:	guess is displayed on screen.
 */

void GetScore( int * blacks, int * whites );
/*
	Post:	The user has entered a score, that score is
		displayed on screen.  blacks + whites <= numholes.
 */

BOOL BoardFull( void );
/*
	Post:	TRUE is returned if the board is completely full,
		(forcing a draw); FALSE is returned otherwise.
 */

void FinishGame( void );
/*
	Post:	The user has been informed that the game is over.
 */

BOOL AnotherGame( void );
/*
	Post:	The user has been asked whether another game is
		wanted, and the result is returned.
 */

void CloseDownMMSupport( void );
/*
	Pre:	You're all done; ready to finish execution.
	Post:	The I/O system is closed down.
 */
