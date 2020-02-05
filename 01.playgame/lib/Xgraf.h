/*
 *	Xgraf.h
 *
 *	Public header file for Xgraf.
 */

/* X_H is declared in X11/X.h, so if it ain't declared, neither is Window! */
#ifndef X_H
typedef unsigned long Window;
#endif

/* Values for pen state variable */
typedef enum {
	OnPixel, OffPixel, InvertPixel
} PlotAttribute;

/* Line Styles */
typedef enum {
	LineSolidStyle,
	LineDashStyle,
	LineDoubleDashStyle
} LineStyle;

/* event types */
typedef enum
{
	NoEvent, EventExpose, EventKeypress, EventKeyrelease,
	EventMouseUp, EventMouseDown, EventEnter, EventLeave,
	EventMove, EventResize
} EventType;

/* Event Masks */
typedef enum
{
	MaskNull		= 0,
	MaskKeypress		= 1,
	MaskKeyrelease		= 2,
	MaskMouseUp		= 4,
	MaskMouseDown		= 8,
	MaskEnter		= 16,
	MaskLeave		= 32,
	MaskButtonMotion	= 64,
	MaskAllMotion		= 128,
	MaskResize		= 256
} EventMask;

#define MaskKeys		(MaskKeypress | MaskKeyrelease)
#define MaskMouse		(MaskMouseUp | MaskMouseDown)
#define MaskInOut		(MaskEnter | MaskLeave)

/* Fill styles for FillRectangle and FillArc */
typedef enum {
	SolidStyle, FineVertStyle, FineHorizStyle,
	ThickVertStyle, ThickHorizStyle,
	FineLeftDiagStyle, FineRightDiagStyle,
	ThickLeftDiagStyle, ThickRightDiagStyle,
	FineCheckStyle, ThickCheckStyle, GreyStyle
} FillingStyle;

#include "Xcolours.h"

#ifndef X_H
typedef int Font;
#endif

/* general junk */
#define FALSE		0
#define TRUE		1
typedef char BOOL;

typedef char String[200];

typedef struct
{
	char ch;		/* when Event Keypresss/Keyrelease */
	int keycode;
	int keystate;
	int x, y;		/* when Event MouseMove */
	int w, h;		/* when Event Resize */
	int button;		/* when Event MouseDown/MouseUp */
} EventData;

/*
/^#ifdef HASPROTOS
d/endif$
!!prot %
*/

extern void EnquireSystem( int * , int * , double * , double * );
extern void UseSharedColourMap( void );
extern void UseOwnColourMap( void );
extern void UseRGBColourMap( void );
extern void DebugImportant( void );
extern void DebugAll( void );
extern void InitialiseGraf( void );
extern void CloseDownGraf( void );
extern Window RootWin( void );
extern void SetEventMask( EventMask );
extern void SetWBackground( Colour );
extern void SetWMinSize( int , int );
extern void SetWMaxSize( int , int );
extern void SetWBackingStore( void );
extern void SetWSaveUnder( void );
extern void SetWNotShown( void );
extern Window MakeWindow( Window , char * , int , int , int , int );
extern void SelectWindow( Window );
extern void ShowWindow( void );
extern void HideWindow( void );
extern void MoveResizeWindow( int , int , int , int );
extern void MoveWindow( int , int );
extern void ResizeWindow( int , int );
extern void CloseWindow( void );
extern void ClearWindow( void );
extern Font LoadFont( char * );
extern Font DefaultFont( void );
extern void SelectFont( Font );
extern void GetFontName( Font , char * );
extern void CharSize( char , int * , int * );
extern int StringWidth( char * );
extern int FontHeight( void );
extern void StringSize( char * , int * , int * );
extern void SetRGB( Colour , int , int , int );
extern void GetRGB( Colour , int * , int * , int * );
extern void SetColour( Colour );
extern void SetBackColour( Colour );
extern void GetColourName( Colour , char * );
extern Colour FirstColour( void );
extern Colour LastColour( void );
extern int NumColours( void );
extern void AllowNoEvent( void );
extern void GetEvent( Window * , EventType * , EventData * );
extern int EventPending( void );
extern void SetPlotAttribute( PlotAttribute );
extern void SetLineThickness( int );
extern void SetLineStyle( LineStyle );
extern void ResetLineThicknessAndStyle( void );
extern void SetFillStyle( FillingStyle );
extern FillingStyle FirstStyle( void );
extern FillingStyle LastStyle( void );
extern void DrawPoint( int , int );
extern void DrawLine( int , int , int , int );
extern void DrawRectangle( int , int , int , int );
extern void FillRectangle( int , int , int , int );
extern void DrawChar( int , int , char );
extern void DrawString( int , int , char * );
extern void DrawArc( int , int , int , int , int );
extern void DrawCircle( int , int , int );
extern void FillArc( int , int , int , int , int );
extern void FillCircle( int , int , int );
extern void OnCursor( void );
extern void BusyCursor( void );
extern void OffCursor( void );
extern void SetClipList( void );
extern void ClearClipList( void );
extern void FlushOutput( void );

/*
If you wish to access the Display, add:
	extern Display * GetDisplay( void );
at the top of your C file.
*/

