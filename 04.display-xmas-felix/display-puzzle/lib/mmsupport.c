/*
	mmsupport.c:

	I/O module for the game of Mastermind.

	Duncan C. White et al.
	Started 24th July 1992

	C translation, 16/Nov/2000, Duncan C. White
 */


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <Xkeys.h>
#include <Xgraf.h>

#include "misc.h"
#include "mmsupport.h"


#define uchar unsigned int


#define maxguesses		4	/* Not more guesses than this. */

/* Collapse special 16-bit keycodes into these chars */
#define Left			'\377'
#define Right			'\376'
#define Delete			'\375'
#define BackSpace		'\374'
#define Return			'\373'
#define Tab			'\010'


typedef char ColourString[maxcolours+1];

typedef char GuessString[maxholes+1];

typedef struct
{
	GuessString g;
	int b, w;
} GuessAndScore;

typedef GuessAndScore GuessAndScoreList[ maxguesses+1 ];

typedef struct {
	int               numholes;
	int               numcolours;
	ColourString      validcolours;
	int               numguesses;
	BOOL              lastscored;
	BOOL              cursoron;
	int               cursorat;
	GuessAndScoreList gs;
} GameStateObject;

typedef struct {
	Window win;
	int    w, h;
	int    x, y;
} CommonWinData;

typedef struct {
	CommonWinData data;
} NumWinObject;

typedef struct {
	CommonWinData data;
	int           hg;
	int           innerg, outerg;
	int           radius, diam;
} GuessWinObject;

typedef struct {
	CommonWinData data;
	int           pegw, pegh;
	int           hg, thickness;
} ScoreWinObject;

typedef struct {
	CommonWinData  data;
	NumWinObject   numobj;
	GuessWinObject guessobj;
	ScoreWinObject scoreobj;
	int            gap, vg, vscale;
	int            texth;
	Font           textfont;
} BoardWinObject;

typedef struct {
	CommonWinData data;
	char mesg[1024];
} OneMesgWinObject;

typedef struct {
	CommonWinData    data;
	int              nummessages; /* 0, 1 || 2 */
	int              vg, hg;
	int              innerg;
	OneMesgWinObject o1, o2;
	int              texth;
	Font             textfont;
} MesgWinObject;


GameStateObject gameobj;
BoardWinObject  boardobj;
MesgWinObject   mesgobj;
BOOL            ismono;


static void appendint( char * buf, int n );
static void InputScore( int * b, int * w );
static void BoardLayout( void );
static void BoardRelayout( int neww, int newh );
static BOOL odd( int x );
static void MessageLayout( void );
static void MessageRelayout( int neww, int newh );
static void CreateWindows( void );
static void ClearBoardW( void );
static void ClearMessageW( void );
static void AddMessage( char * M );
static void OverrideMessage( char * M );
static void Resize( Window w, EventData data );
static void Redraw( Window w );
static void RedrawBoardW( void );
static void RedrawNumW( void );
static void RedrawGuessW( void );
static void RedrawScoreW( void );
static void RedrawMessageW( void );
static void RedrawMesgW1( void );
static void RedrawMesgW2( void );
static void ShowNum( int row );
static void ShowLastGuess( void );
static void ShowGuess( int N, GuessAndScore gs );
static BOOL IsValidColourChar( char ch );
static void GetColourAndOpposite( char ch, Colour * col, Colour * oppcol );
static void ShowColour( char ch, int row, int column );
static void ShowScore( int N, GuessAndScore gs );
static void ShowPeg( Colour c, int row, int column );
static void ShowMesg( OneMesgWinObject m );
static void FillWinWhite( CommonWinData data );
static void SetWhiteFill( void );
static void FillWinSteelBlue( CommonWinData data );
static void SetSteelBlueFill( void );
static BOOL GetYesNo( void );
static void InputGuess( char * guess );
static void Retreat( int * cursorat );
static void Advance( int * pos );
static uchar GetChar( void );


int Random( int n )
{
	assert( n >= 1 );

    	return random() % n;

} /* Random */


void InitialiseMMSupport( int n, char *vc )
{
	int i;

	srandom( getpid() ^ time(NULL) );

	gameobj.numholes   = n;
	gameobj.numcolours = strlen(vc);

	assert( gameobj.numholes <= maxholes );

	assert( gameobj.numcolours <= maxcolours );

	for( i = 0; i < gameobj.numcolours; i++ )
	{
		gameobj.validcolours[i] = toupper(vc[i]);
	}
	gameobj.validcolours[i] = '\0';

	UseSharedColourMap();
	InitialiseGraf();

	ismono = gameobj.numcolours == 2;

	BoardLayout();
	MessageLayout();
	CreateWindows();

} /* InitialiseMMSupport */


void StartGame( void )
{
	ClearMessageW();
	ClearBoardW();

	gameobj.numguesses = 0;

} /* StartGame */


#define appendstr( buf, str ) strcat( (buf), (str) )


static void appendint( char *buf, int n )
{
	String str;

	sprintf( str, "%d", n );
	strcat( buf, str );
}


void GetGuess( char *guess )
{
	String mesg;

	gameobj.numguesses++;

	strcpy( mesg, "Please enter your " );
	appendstr( mesg, gameobj.numguesses == 1 ?  "first" : "next" );
	appendstr( mesg, " guess (and press return)" );

	AddMessage( mesg );

	InputGuess( guess );

} /* GetGuess */


void PutScore( int blacks, int whites )
{
	String str;
	GuessAndScore *gas;

	assert( blacks + whites <= gameobj.numholes );

	strcpy(str, "You get " );
	appendint(str, blacks );
	appendstr(str, " black(s) and " );
	appendint(str, whites );
	appendstr(str, " white(s)" );

	AddMessage(str);

	gameobj.lastscored = TRUE;

	gas = &gameobj.gs[gameobj.numguesses];
	gas->b = blacks;
	gas->w = whites;

	ShowScore(gameobj.numguesses, *gas );

	FlushOutput();

} /* PutScore */


void PutGuess( char *guess )
{
	int i;
	GuessAndScore *gas;

	gameobj.numguesses++;

	gas = &gameobj.gs[gameobj.numguesses];

	gameobj.cursoron   = FALSE;
	gameobj.cursorat   = 0;
	gameobj.lastscored = FALSE;

	for( i = 0; i < gameobj.numholes; i++ )
	{
		char c = toupper(guess[i]);
		assert( IsValidColourChar(c) );
		gas->g[i] = c;
	}

	ShowLastGuess();

} /* PutGuess */


void GetScore( int *blacks, int *whites )
{
	String str;
	GuessAndScore *gas;

	strcpy( str, "Please enter the score (blacks, whites, then return)" );

	AddMessage(str);

	InputScore( blacks, whites );

	assert( *blacks + *whites <= gameobj.numholes );

	gameobj.lastscored = TRUE;

	gas = &gameobj.gs[gameobj.numguesses];

	gas->b = *blacks;
	gas->w = *whites;

	ShowScore( gameobj.numguesses, *gas );

	strcpy( str, "Score is " );
	appendint( str, *blacks );
	appendstr( str, " black(s) and " );
	appendint( str, *whites );
	appendstr( str, " white(s)" );

	OverrideMessage(str);

	FlushOutput();

} /* GetScore */


typedef enum { Start, GotBlacks, GotBoth } StateEnum;


/* "3 2 return" means 3 blacks, 2 whites
   Delete is allowed
 */
static void InputScore( int *b, int *w )
{
	uchar	  ch;
	int       n;
	BOOL      over;
	StateEnum state;
	String    str;

	over  = FALSE;
	state = Start;

	while( !over )
	{
		ch = GetChar();

		if( isspace(ch) || isdigit(ch) )
		{
			n = ( isspace(ch) || ch == '0' ) ? 0 : ch - '0';

			strcpy(str, "Score is " );

			switch( state )
			{
			case Start:
				*b = n; state = GotBlacks;
				appendint(str, *b );
				appendstr(str, " black(s) " );
				break;

			case GotBlacks:
			case GotBoth:
				*w = n; state = GotBoth;
				appendint(str, *b );
				appendstr(str, " black(s) and " );
				appendint(str, *w );
				appendstr(str, " white(s)" );
			}

			OverrideMessage(str);

		} else if( ch == 'Q' )
		{

			CloseDownGraf();
			exit(0);

		} else if( ch == Delete ||  ch == BackSpace )
		{
			strcpy( str, "Score is " );
			switch( state )
			{
			case Start:
			case GotBlacks:
				state = Start;
				appendstr( str, "0 blacks" );
				break;
			case GotBoth:
				state = GotBlacks;
				appendint( str, *b );
				appendstr( str, " black(s)" );
			}

			OverrideMessage(str);
		} else if( ch == Return )
		{
			over = state == GotBoth;
		}

	}

} /* InputScore */


BOOL BoardFull( void )
{
	return gameobj.lastscored && gameobj.numguesses == maxguesses;

} /* BoardFull */


void FinishGame( void )
{
	String str;

	strcpy( str, "Game " );
	if( gameobj.numguesses > 0
	&&  gameobj.gs[gameobj.numguesses].b == gameobj.numholes )
	{
		appendstr(str, "finished in " );
	} else
	{
		appendstr(str, "abandoned after " );
	} /* if( */
	appendint( str, gameobj.numguesses );
	appendstr( str, " guesses" );

	AddMessage(str);

} /* FinishGame */


BOOL AnotherGame( void )
{
	AddMessage( "Would you like another game? ");
	return GetYesNo();

} /* AnotherGame */


void CloseDownMMSupport( void )
{

	CloseDownGraf();

} /* CloseDownMMSupport */


/* ----------------------- Private Procedures ----------------------- */


/* Set up Board window */

static void BoardLayout( void )
{
#define BoardWinX	100
#define BoardWinY	100

	String str;
	int dummy;

        /* Load a nice font for the Board window */
        strcpy( str, "-adobe-new century schoolbook-bold-r-normal--10-*" );
        boardobj.textfont = LoadFont( str );
        SelectFont( boardobj.textfont );
        StringSize( str , &dummy , &boardobj.texth );

        boardobj.vg = 5 ;
        boardobj.gap = 10 ;

        /* Horizontal init. of boardobj.numobj */

	boardobj.numobj.data.x = boardobj.gap ;
	boardobj.numobj.data.w = 30 ;

	boardobj.guessobj.hg = 13 ;
	boardobj.guessobj.innerg = 7 ;
	boardobj.guessobj.outerg = 2 ;
	boardobj.guessobj.diam = boardobj.texth + 2 * boardobj.guessobj.innerg;
	boardobj.guessobj.radius = boardobj.guessobj.diam / 2 ;

	/* in boardobj */
	boardobj.vscale = boardobj.guessobj.diam + boardobj.vg ;

	boardobj.guessobj.data.x = boardobj.numobj.data.x +
		boardobj.numobj.data.w + boardobj.gap ;
	boardobj.guessobj.data.y = boardobj.gap ;
	boardobj.guessobj.data.w = ( boardobj.guessobj.diam +
		boardobj.guessobj.hg ) * gameobj.numholes +
		boardobj.guessobj.hg ;
	boardobj.guessobj.data.h = boardobj.vscale * maxguesses +
		boardobj.vg ;

	/* Vertical init. for boardobj.NumObj */

	boardobj.numobj.data.y = boardobj.gap ;
	boardobj.numobj.data.h = boardobj.guessobj.data.h ;

	boardobj.scoreobj.hg = 6 ;
	boardobj.scoreobj.pegh = boardobj.guessobj.diam ;
	boardobj.scoreobj.pegw = 11 ;
	boardobj.scoreobj.thickness = max( boardobj.scoreobj.pegw / 4 , 2 );
	if( odd( boardobj.scoreobj.pegw ) != odd( boardobj.scoreobj.thickness) )
	{
		boardobj.scoreobj.thickness++;
	}

	boardobj.scoreobj.data.x = boardobj.guessobj.data.x +
		boardobj.guessobj.data.w +
		boardobj.gap ;
	boardobj.scoreobj.data.y = boardobj.gap ;
	boardobj.scoreobj.data.h = boardobj.guessobj.data.h ;
	boardobj.scoreobj.data.w = ( boardobj.scoreobj.pegw +
		boardobj.scoreobj.hg ) * gameobj.numholes +
		boardobj.scoreobj.hg ;

	boardobj.data.w = boardobj.gap * 4 + boardobj.numobj.data.w +
			boardobj.guessobj.data.w + boardobj.scoreobj.data.w ;
	boardobj.data.h = 2 * boardobj.gap + boardobj.guessobj.data.h ;
	boardobj.data.x = BoardWinX ;
	boardobj.data.y = BoardWinY ;

} /* BoardLayout */


static void BoardRelayout( int neww, int newh )
/*
 *	 Relayout Board window and it's subwindows
 *
 * first, distribute the overall size by giving 5% of the total width
 * to the 4 horizontal gaps, 10% to the number subwindow,
 * 60%; the guess subwindow, and the remaining 25% to the score subwindow.
 *
 * then do more detailed allocation of the space into gaps, pegs, disks etc.
 */

#define	GapPercent	10
#define	NumWPercent	10
#define	GuessWPercent	55
#define	ScoreWPercent	25

{
	printf ( "Board Relayout: resize to (%d x %d) : relaying out\n",
		neww, newh );

	boardobj.gap = ( GapPercent * neww / 100 ) / 4 ;
	boardobj.data.w = neww ;
	boardobj.data.h = newh ;

	boardobj.numobj.data.x = boardobj.gap ;
	boardobj.numobj.data.y = boardobj.gap ;
	boardobj.numobj.data.w = NumWPercent * neww / 100 ;
	boardobj.numobj.data.h = newh - 2 * boardobj.gap ;

	SelectWindow( boardobj.numobj.data.win );
	MoveResizeWindow( boardobj.numobj.data.x,
		boardobj.numobj.data.y, boardobj.numobj.data.w,
		boardobj.numobj.data.h );

	boardobj.guessobj.data.x = boardobj.numobj.data.x +
		boardobj.numobj.data.w + boardobj.gap ;
	boardobj.guessobj.data.y = boardobj.gap ;
	boardobj.guessobj.data.w = GuessWPercent * neww / 100 ;
	boardobj.guessobj.data.h = boardobj.numobj.data.h ;

	SelectWindow( boardobj.guessobj.data.win );
	MoveResizeWindow( boardobj.guessobj.data.x,
		boardobj.guessobj.data.y, boardobj.guessobj.data.w,
		boardobj.guessobj.data.h );

	/* -------------- VERTICAL ----------------- */

	/*
	 * let all the disks occupy 95 % of the Height
	 */
	boardobj.guessobj.diam = ((95 * boardobj.guessobj.data.h ) / 100 )
			/ maxguesses ;
	boardobj.guessobj.radius = boardobj.guessobj.diam / 2 ;
	boardobj.guessobj.innerg = ( boardobj.guessobj.diam -
		boardobj.texth ) / 2 ;

	/*
	 * let the VGs occupy the remaining (5%) of the Height
	 */
	boardobj.vg = ( boardobj.guessobj.data.h -
		maxguesses * boardobj.guessobj.diam ) /
		( 1 + maxguesses );
	boardobj.guessobj.outerg = boardobj.vg / 3 ;
	boardobj.vscale = boardobj.guessobj.diam + boardobj.vg ;

	/* M4
	str (" BoardRelayout :"); Int ( boardobj.Gap ); Int ( Data.H );
	Int ( MaxGuesses ); Int ( boardobj.VG ); Int ( boardobj.guessobj.OuterG ); nl ();
	Int ( boardobj.guessobj.Diam ); Int ( boardobj.VScale ); Int ( boardobj.guessobj.InnerG ); nl ();
	*/

	/* -------------- HORIZONTAL ----------------- */

	/*
	 * let all the HGs occupy the remaining space.
	 */
	boardobj.guessobj.hg = ( boardobj.guessobj.data.w -
		boardobj.guessobj.diam * gameobj.numholes ) /
		(1 + gameobj.numholes );

	boardobj.scoreobj.data.x = boardobj.guessobj.data.x +
		boardobj.guessobj.data.w + boardobj.gap ;
	boardobj.scoreobj.data.y = boardobj.gap ;
	boardobj.scoreobj.data.w = ( ScoreWPercent * neww ) / 100 ;
	boardobj.scoreobj.data.h = boardobj.numobj.data.h ;

	SelectWindow( boardobj.scoreobj.data.win );
	MoveResizeWindow( boardobj.scoreobj.data.x, boardobj.scoreobj.data.y,
		boardobj.scoreobj.data.w, boardobj.scoreobj.data.h );

	/* --------------- VERTICAL ------------------ */

	boardobj.scoreobj.pegh = boardobj.guessobj.diam ;

	/* -------------- HORIZONTAL ----------------- */

	/*
	 * let the pegs occupy 75 % of the horizontal space
	 */
	boardobj.scoreobj.pegw = ((75 * boardobj.scoreobj.data.w ) / 100 )
			/ gameobj.numholes ;

	/*
	 * give the remaining (25%) horizontal space to HGs
	 */
	boardobj.scoreobj.hg = ( boardobj.scoreobj.data.w -
		boardobj.scoreobj.pegw * gameobj.numholes ) /
		(1 + gameobj.numholes );

	boardobj.scoreobj.thickness = max( boardobj.scoreobj.pegw / 4 , 2 );
	if( odd( boardobj.scoreobj.pegw ) != odd(boardobj.scoreobj.thickness) )
	{
		boardobj.scoreobj.thickness++;
	}

} /* BoardRelayout */


static BOOL odd( int x )
{
	return (x%2) == 1;
}


static void MessageLayout( void )

#define	MessageWinWidth 400
#define	MessageWinX	600
#define	MessageWinY	300

{
	String str;
	int    dummy;

	/* Set up Mesage window */

	/* Load a nice font for the Message window */
	strcpy( str, "-adobe-new century schoolbook-bold-r-normal--12-*" );
	mesgobj.textfont = LoadFont ( str );
	SelectFont( mesgobj.textfont );
	StringSize( str , &dummy , &mesgobj.texth );

	mesgobj.vg = mesgobj.texth ;
	mesgobj.hg = 10 ;
	mesgobj.innerg = mesgobj.vg / 2 ;

	mesgobj.o1.data.w = MessageWinWidth - 2 * mesgobj.hg ;
	mesgobj.o1.data.h = 2 * mesgobj.innerg + mesgobj.texth ;
	mesgobj.o1.data.x = mesgobj.hg ;
	mesgobj.o1.data.y = mesgobj.vg ;

	mesgobj.o2.data.w = mesgobj.o1.data.w ;
	mesgobj.o2.data.h = mesgobj.o1.data.h ;
	mesgobj.o2.data.x = mesgobj.o1.data.x ;
	mesgobj.o2.data.y = mesgobj.o1.data.y + mesgobj.o1.data.h + mesgobj.vg ;

	mesgobj.data.x = MessageWinX ;
	mesgobj.data.y = MessageWinY ;
	mesgobj.data.w = MessageWinWidth ;
	mesgobj.data.h = mesgobj.o1.data.h + mesgobj.o2.data.h +
		3 * mesgobj.vg ;

} /* MessageLayout */


static void MessageRelayout( int neww, int newh )

/* Reconfigure Message window */

#define	HGapPercent 10

{
	printf ( "Message Relayout: resize to (%d x %d) : relaying out\n",
		neww, newh );

        /*
         * distribute space among gaps , with Inner Gaps
         * getting half as much as Full Gaps
         * (3 full gaps and 4 inner gaps == 5 full gaps )
         */
        mesgobj.vg = ( newh - 2 * mesgobj.texth ) / 5 ;
        mesgobj.innerg = mesgobj.vg / 2 ;
        /*
         * distribute space by giving 10 % of the total width
         * to the 2 horizontal gaps , and the remaining (90%)
         * to the message subwindow.
         */
        mesgobj.hg = (( neww * HGapPercent ) / 100 ) / 2 ;

	mesgobj.o1.data.x = mesgobj.hg ;
	mesgobj.o1.data.y = mesgobj.vg ;
	mesgobj.o1.data.w = neww - 2 * mesgobj.hg ;
	mesgobj.o1.data.h = 2 * mesgobj.innerg + mesgobj.texth ;
	SelectWindow( mesgobj.o1.data.win );
	/*
	HideWindow ();
	*/
	MoveResizeWindow( mesgobj.o1.data.x, mesgobj.o1.data.y,
		mesgobj.o1.data.w , mesgobj.o1.data.h );
	/*
	ShowWindow ();
	*/

	mesgobj.o2.data.x = mesgobj.o1.data.x ;
	mesgobj.o2.data.y = mesgobj.o1.data.y + mesgobj.o1.data.h + mesgobj.vg ;
	mesgobj.o2.data.w = mesgobj.o1.data.w ;
	mesgobj.o2.data.h = mesgobj.o1.data.h ;
	SelectWindow( mesgobj.o2.data.win );
	/*
	HideWindow();
	*/
	MoveResizeWindow( mesgobj.o2.data.x, mesgobj.o2.data.y,
		mesgobj.o2.data.w , mesgobj.o2.data.h );
	/*
	ShowWindow();
	*/

	mesgobj.data.w = neww ;
	mesgobj.data.h = newh ;

} /* MessageRelayout */


static void CreateWindows( void )
{
	String title;
	Window xw;

	SetEventMask( MaskKeypress + MaskResize );
	SetWMinSize( boardobj.data.w, boardobj.data.h );
	SetWSaveUnder();
	strcpy( title, "Board" );
	boardobj.data.win = MakeWindow( RootWin(), title,
		boardobj.data.x, boardobj.data.y,
		boardobj.data.w, boardobj.data.h );
	xw = boardobj.data.win ;

	SetEventMask( MaskKeypress );
	SetWBackingStore();
	strcpy( title, "" );
	boardobj.numobj.data.win = MakeWindow( xw, title,
		boardobj.numobj.data.x, boardobj.numobj.data.y,
		boardobj.numobj.data.w, boardobj.numobj.data.h );

	strcpy( title, "" );
	SetEventMask( MaskKeypress );
	SetWBackingStore();
	boardobj.guessobj.data.win = MakeWindow( xw, title,
		boardobj.guessobj.data.x, boardobj.guessobj.data.y,
		boardobj.guessobj.data.w, boardobj.guessobj.data.h );

	SetEventMask( MaskKeypress );
	SetWBackingStore();
	strcpy( title, "" );
	boardobj.scoreobj.data.win = MakeWindow( xw, title,
		boardobj.scoreobj.data.x, boardobj.scoreobj.data.y,
		boardobj.scoreobj.data.w, boardobj.scoreobj.data.h );

	SetEventMask( MaskKeypress + MaskResize );
	SetWMinSize( mesgobj.data.w, mesgobj.data.h );
	SetWSaveUnder();
	strcpy( title, "Messages" ) ;
	mesgobj.data.win = MakeWindow( RootWin(), title,
		mesgobj.data.x, mesgobj.data.y,
		mesgobj.data.w, mesgobj.data.h );
	xw = mesgobj.data.win ;

	SetEventMask( MaskKeypress );
	SetWBackingStore();
	strcpy( title, "" );
	mesgobj.o1.data.win = MakeWindow( xw, title,
		mesgobj.o1.data.x, mesgobj.o1.data.y,
		mesgobj.o1.data.w, mesgobj.o1.data.h );

	SetEventMask( MaskKeypress );
	SetWBackingStore();
	strcpy( title, "" );
	mesgobj.o2.data.win = MakeWindow( xw, title,
		mesgobj.o2.data.x, mesgobj.o2.data.y,
		mesgobj.o2.data.w, mesgobj.o2.data.h );

	SetPlotAttribute( OnPixel );

} /* CreateWindows */


static void ClearBoardW( void )
{
	gameobj.numguesses = 0;
	gameobj.lastscored = FALSE;
	gameobj.cursoron   = FALSE;
	gameobj.cursorat   = 0;

	RedrawBoardW();
	RedrawNumW  ();
	RedrawGuessW();
	RedrawScoreW();

} /* ClearBoardW */


static void ClearMessageW( void )
{
	mesgobj.nummessages = 0;
	strcpy( mesgobj.o1.mesg, "" );
	strcpy( mesgobj.o2.mesg, "" );

	RedrawMessageW();
	RedrawMesgW1();
	RedrawMesgW2();

} /* ClearMessageW */


static void AddMessage( char *M )
/*
	Two line scrolling system.
 */

{
	if( mesgobj.nummessages == 0 )
	{
		strcpy( mesgobj.o1.mesg, M ) ; ShowMesg(mesgobj.o1);
		strcpy( mesgobj.o2.mesg, "" ); ShowMesg(mesgobj.o2);
		mesgobj.nummessages	= 1;
	} else
	{
		if( mesgobj.nummessages == 2 )
		{
			strcpy( mesgobj.o1.mesg, mesgobj.o2.mesg );
			ShowMesg(mesgobj.o1);
		}
		mesgobj.nummessages	= 2;
		strcpy( mesgobj.o2.mesg, M );
		ShowMesg(mesgobj.o2);
	}

} /* AddMessage */


static void OverrideMessage( char *M )
{
	if( mesgobj.nummessages == 0 )
	{
		strcpy( mesgobj.o1.mesg, M );
		ShowMesg(mesgobj.o1);
		mesgobj.nummessages	= 1;
	} else
	{
		mesgobj.nummessages	= 2;
		strcpy( mesgobj.o2.mesg, M );
		ShowMesg(mesgobj.o2);
	}
} /* OverrideMessage */


/* --------------------------- Resize Procedure ------------------------- */


static void Resize( Window w, EventData data )
{
	SelectWindow(w);
	SetPlotAttribute(OnPixel);

	/* PutLn( "EventResize received" ); */

	if( w == boardobj.data.win )
	{
		BoardRelayout  (data.w, data.h);
	} else if( w == mesgobj.data.win )
	{
		MessageRelayout(data.w, data.h);
	}
} /* Resize */


/* ------------------------- Redraw Procedures --------------------------- */


static void Redraw( Window w )
{
	SelectWindow(w);
	SetPlotAttribute(OnPixel);
	SetClipList();

	if(      w == boardobj         .data.win ) { RedrawBoardW  (); }
	else if( w == boardobj.numobj  .data.win ) { RedrawNumW    (); }
	else if( w == boardobj.guessobj.data.win ) { RedrawGuessW  (); }
	else if( w == boardobj.scoreobj.data.win ) { RedrawScoreW  (); }
	else if( w == mesgobj          .data.win ) { RedrawMessageW(); }
	else if( w == mesgobj.o1       .data.win ) { RedrawMesgW1  (); }
	else if( w == mesgobj.o2       .data.win ) { RedrawMesgW2  (); }
	else
	{
		fprintf(stderr,"Redraw: internal error - unknown window\n");
		exit(1);
	}

	ClearClipList();

} /* Redraw */


static void RedrawBoardW( void )
{
	FillWinWhite(boardobj.data);

} /* RedrawBoardW */


static void RedrawNumW( void )
{
	int i;

	FillWinSteelBlue( boardobj.numobj.data );

	for( i = 0; i <= maxguesses; i++ )
	{
		ShowNum(i);
	}

} /* RedrawNumW */


static void RedrawGuessW( void )
{
	int i;

	FillWinSteelBlue( boardobj.guessobj.data );

	for( i = 1; i <= gameobj.numguesses; i++ )
	{
		ShowGuess( i, gameobj.gs[i] );
	}

} /* RedrawGuessW */


static void RedrawScoreW( void )
{
	int i;

	FillWinSteelBlue( boardobj.scoreobj.data );

	for( i = 1; i < gameobj.numguesses; i++ )
	{
		ShowScore( i, gameobj.gs[i] );
	}

	if( gameobj.numguesses > 0 && gameobj.lastscored )
	{
		ShowScore(gameobj.numguesses, gameobj.gs[gameobj.numguesses]);
	}

} /* RedrawScoreW */


static void RedrawMessageW( void )
{
	FillWinWhite( mesgobj.data );

} /* RedrawMessageW */


static void RedrawMesgW1( void )
{
	ShowMesg( mesgobj.o1 );
} /* RedrawMesgW1 */


static void RedrawMesgW2( void )
{
	ShowMesg( mesgobj.o2 );
} /* RedrawMesgW2 */


/* --------------------------- Drawing Procedures -------------------------- */


static void ShowNum( int row )
{
	int leftx, boty;
	int rightx, topy;
	String mesg;
	int mesgw, dummy;

        SelectFont( boardobj.textfont ); 

	strcpy( mesg, "" ); 
	appendint( mesg, row ); 

	StringSize( mesg , &mesgw , &dummy ); 

	leftx = ( boardobj.numobj.data.w - mesgw ) / 2 ; 
	rightx = leftx + mesgw - 1 ; 
	topy = boardobj.vg + boardobj.vscale * ( row - 1 ) + 
		   ( boardobj.guessobj.diam - boardobj.texth ) / 2 ; 
	boty = topy + boardobj.texth ; 

	SelectWindow( boardobj.numobj.data.win ); 
	SetFillStyle( SolidStyle ); 

	if( ismono )
	{
		SetColour( Black ); 
		FillRectangle( leftx - 3, topy - 3, 
			       rightx + 3, boty + 3 ); 
	}
	SetColour( White ); 
	DrawString( leftx, boty, mesg ); 

} /* ShowNum */


static void ShowLastGuess( void )
{
        ShowGuess( gameobj.numguesses, gameobj.gs[ gameobj.numguesses ] ); 

} /* ShowLastGuess */


static void ShowGuess( int N, GuessAndScore gs )
{
	int i;

	SelectWindow( boardobj.guessobj.data.win );

	for( i = 0; i < gameobj.numholes; i++ )
	{
		ShowColour( gs.g[i], N, i );
	}

} /* ShowGuess */


static BOOL IsValidColourChar( char ch )
{
	int i;

	for( i = 0; i < gameobj.numcolours; i++ )
	{
		if( ch == gameobj.validcolours[i] )
		{
			return TRUE;
		}
	}

	return FALSE;

} /* IsValidColourChar */


static void GetColourAndOpposite( char ch, Colour *col, Colour *oppcol )
{
	*oppcol = Black;
	if( ismono )
	{
		*col = White;
	} else
	{
		switch( ch )
		{
		case 'B':	*col = Blue; *oppcol = White; break;
		case 'R':	*col = Red; break;
		case 'G':	*col = Green; break;
		case 'Y':	*col = Yellow; break;
		case 'P':	*col = Violet; break;
		case 'C':	*col = Cyan; break;
		case 'O':	*col = Orange; break;
		case 'W':	*col = White ; break;
		default:	*col = White; break;
		}
	}
} /* GetColourAndOpposite */


#define DotSize 	2 

static void ShowColour( char ch, int row, int column )
{
	int halfw;
	int leftx, topy;
	int circx, circy;
	int rightx, boty;
	int charw, charh;
	Colour col, oppcol;

        SelectFont( boardobj.textfont ); 

	leftx = boardobj.guessobj.hg + ( boardobj.guessobj.diam +
		boardobj.guessobj.hg ) * column ; 
	topy = boardobj.vg + boardobj.vscale * ( row - 1 ); 

	halfw = boardobj.guessobj.radius + boardobj.guessobj.outerg ; 
	circx = leftx + halfw ; 
	circy = topy + halfw ; 

	rightx = circx + halfw ; 
	boty = circy + halfw ; 

	SelectWindow( boardobj.guessobj.data.win ); 
	SetSteelBlueFill(); 
	FillRectangle( leftx , topy , rightx , boty ); 

	if( ch == ' ' )
	{
		SetColour( White ); 
		SetFillStyle( SolidStyle ); 
		FillCircle( circx , circy , DotSize ); 
	} else
	{
		GetColourAndOpposite( ch, &col, &oppcol ); 

		SetColour( oppcol ); 
		SetFillStyle ( SolidStyle ); 
		FillCircle( circx , circy , boardobj.guessobj.radius ); 

		SetColour( col ); 
		FillCircle( circx , circy , boardobj.guessobj.radius -2 ); 

		CharSize( ch , &charw , &charh ); 

		SetColour( oppcol ); 
		DrawChar( circx + 1 - charw / 2 , 
			 circy - 1 + charh / 2 , ch ); 

	}

	if( row == gameobj.numguesses && gameobj.cursoron 
	&& column == gameobj.cursorat ) 
	{
		SetColour( Black ); 
		DrawRectangle( leftx , topy , rightx , boty ); 
		DrawRectangle( leftx + 1 , topy + 1 , 
			      rightx - 1 , boty - 1 ); 
	}

} /* ShowColour */


static void ShowScore( int N, GuessAndScore gs )
{
	int i, j;

	j = 0;

	for( i = 0; i < gs.b; i++ )
	{
		ShowPeg( Black, N, j++ );
	}

	for( i = 0; i < gs.w ; i++ )
	{
		ShowPeg( White, N, j++ );
	}

} /* ShowScore */


static void ShowPeg( Colour c, int row, int column )
{
	int leftx, rightx;
	int topy, boty;
	int x1;

	SelectWindow( boardobj.scoreobj.data.win ); 
	SetColour( c ); 
	SetFillStyle( SolidStyle ); 

	leftx = boardobj.scoreobj.hg + ( boardobj.scoreobj.pegw +
		boardobj.scoreobj.hg ) * column ; 
	rightx = leftx + boardobj.scoreobj.pegw - 1 ; 
	topy = boardobj.vg + boardobj.vscale * ( row - 1 ); 
	boty = topy + boardobj.scoreobj.pegh - 1 ; 
	x1 = leftx + ( boardobj.scoreobj.pegw -
		boardobj.scoreobj.thickness ) / 2 ; 

	FillRectangle( leftx , topy , 
		      rightx , topy + boardobj.scoreobj.thickness - 1 ); 
	FillRectangle( x1 , topy , x1 + boardobj.scoreobj.thickness - 1 ,
		      boty ); 
 
} /* ShowPeg */


static void ShowMesg(OneMesgWinObject m )
{
	int strw, dummy;
	int leftx, boty;
	int rightx, topy;

        SelectFont( mesgobj.textfont ); 

	if( m.mesg[0] != '\0' )
	{
		StringSize( m.mesg , &strw , &dummy ); 

		FillWinSteelBlue( m.data ); 

		SetColour( White ); 
		SetFillStyle( SolidStyle ); 

		leftx = ( m.data.w - strw ) / 2 ; 
		rightx = leftx + strw - 1 ; 
		topy = mesgobj.innerg ; 
		boty = topy + mesgobj.texth ; 

		if( ismono )
		{
			SetColour( Black ); 
			FillRectangle( leftx - 3 , topy - 3 , 
				      rightx + 3 , boty + 3 ); 
		}
		SetColour( White ); 
		DrawString( leftx , boty , m.mesg ); 
	} else
	{
		FillWinWhite( m.data ); 
	}
} /* ShowMesg */


static void FillWinWhite( CommonWinData data )
{
	SelectWindow( data.win );
	SetWhiteFill();
	FillRectangle( 0, 0, data.w-1, data.h-1 );
} /* FillWinWhite */


static void SetWhiteFill( void )
{
	SetColour(White);
	SetFillStyle(SolidStyle);
} /* SetWhiteFill */


static void FillWinSteelBlue( CommonWinData data )
{
	SelectWindow( data.win );
	SetSteelBlueFill();
	FillRectangle( 0, 0, data.w-1, data.h-1 );
} /* FillWinSteelBlue */


static void SetSteelBlueFill( void )
{
	if( ismono )
	{
		SetFillStyle(GreyStyle);
		SetColour(White);
		SetBackColour(Black);
	} else
	{
		SetFillStyle(SolidStyle);
		SetColour(SteelBlue);
	}
} /* SetSteelBlueFill */


/* ----------------------- Input And Event Handling ----------------------- */


static BOOL GetYesNo( void )
{
	uchar ch;

	do {
		ch = GetChar();
	} while( ch != 'Y' && ch != 'N' && ch != 'Q' );

	return ch == 'Y';

} /* GetYesNo */


static void InputGuess( char *guess )
{
	uchar ch;
	int i;
	int entered;
	BOOL over;

	gameobj.cursoron = TRUE ; 
	gameobj.cursorat = 0 ; 
	entered = 0 ; 
	gameobj.lastscored = FALSE ; 

	for( i = 0; i < gameobj.numholes; i++ )
	{
		gameobj.gs[gameobj.numguesses].g[ i ] = ' ' ; 
	}

	ShowLastGuess (); 

	over = FALSE ; 

	do
	{
		ch = GetChar(); 

		if( IsValidColourChar( ch ) )
		{
			if( gameobj.gs[gameobj.numguesses].
				g[ gameobj.cursorat ] == ' ' )
			{
				entered++;
			}

			gameobj.gs[gameobj.numguesses].
				g[ gameobj.cursorat ] = ch ; 

			Advance( &(gameobj.cursorat) ); 

		} else if( ch == 'Q' )
		{
			CloseDownGraf(); 
			exit(0); 
		} else if( ch == Delete || ch == BackSpace ) 
		{
			Retreat( &gameobj.cursorat ); 

			if( gameobj.gs[gameobj.numguesses].
				g[ gameobj.cursorat ] != ' ' )
			{
				entered--;
				gameobj.gs[gameobj.numguesses].
					g[ gameobj.cursorat ] = ' ' ; 
			}

		} else if( ch == Return )
		{
			over = entered = gameobj.numholes ; 
		} else if( ch == ' ' || ch == '+' || ch == Right ) 
		{
			Advance( &(gameobj.cursorat) ); 
		} else if( ch == Tab || ch == '-' || ch == Left ) 
		{ 
			Retreat( &gameobj.cursorat ); 
		}

		ShowLastGuess(); 

	} while( ! over );

	for( i = 0; i < gameobj.numholes; i++ )
	{
		guess[ i ] = gameobj.gs[gameobj.numguesses].g[ i ]; 
	}

	gameobj.cursoron = FALSE ; 

	ShowLastGuess(); 

} /* InputGuess */


static void Retreat( int *cursorat )
{
	if( *cursorat <= 0 )
	{
		*cursorat = gameobj.numholes-1;
	} else
	{
		(*cursorat)--;
	}
} /* Retreat */


static void Advance( int *pos )
{
	if( ++(*pos) >= gameobj.numholes )
	{
		*pos = 0;
	}
} /* Advance */


static uchar GetChar( void )
/*
	Here's the event loop
 */
{
	Window    win;
        EventType event;
	EventData data;
        BOOL      over;
	uchar     ch = 0;

	over = FALSE;

	do {
		GetEvent( &win, &event, &data );

		if( event == EventExpose )
		{
			Redraw(win);
		} else if( event == EventResize )
		{
			Resize(win, data);
		} else if( event == EventKeypress )
		{
			over = TRUE;
			switch( data.keycode )
			{
			case XkeyLeft:
				ch = Left;
				break;
			case XkeyRight:
				ch = Right;
				break;
			case XkeyDelete:
				ch = Delete;
				break;
			case XkeyBackSpace:
				ch = BackSpace;
				break;
			case XkeyReturn:
				ch = Return;
				break;
			default:
				if( data.ch != '\0' )
				{
					ch = toupper(data.ch);
				} else
				{
					over = FALSE;
				}
			}
		}

	} while( ! over );

	return ch;

} /* GetChar */


