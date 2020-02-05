/*
 *	Xgraf.c
 *
 *	X11 implementation of Xgraf (the simple Modula-2 graphics library)
 */


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "Xcolours.h"
#include "Xgraf.h"
#include "Xgrafprivate.h"


extern void		exit();
extern GC		XCreateGC();


typedef enum { debug_none, debug_important, debug_all } debug_type;


#define WS_MIN		1
#define WS_MAX		2
#define WS_BACKINGSTORE	4
#define WS_SAVEUNDER	8
#define WS_MAPWIN	16
#define WS_BGCOL	32


typedef struct
{
	int	mask;
	int	minw, minh;
	int	maxw, maxh;
	int	eventmask;
	Colour	bgcol;
} windowsettings;


typedef struct
{
	int		size, used;
	int		current;
	char		**name;			/* fontnames */
	XFontStruct	**font;			/* all the fonts */
} fontstruct;


typedef struct
{
	Window	win;
	int	w, h;
} windowstruct;


typedef struct
{
	int		size, used;
	windowstruct	*data;			/* array of window sizes */
} winsizedata;


static Window		rootwin;		/* the root window */
static Display		*dpy;			/* the display */
static int		thescreen;		/* the screen */
static Window		currentwin;		/* the window */
static winsizedata	winsize;		/* window sizes */
static GC		gc;			/* graphics context */
static PlotAttribute	current_pen;		/* colour of current pen */
static int		planes;			/* no. of colour planes */
static int		maxcolours;		/* max possible # colours */
static int		ncolours;		/* # of colour names above */
static int		tablesize;		/* size of colour name table */
static unsigned long	*colourvector;		/* vector of pixel values */
static Colormap		cmap;			/* the colour map */
static Cursor		user_cursor;		/* the users cursor */
static Cursor		busy_cursor;		/* the busy cursor */
static windowsettings	wsettings;		/* per window settings */
static unsigned long	fg, bg;			/* foregound and background */
static Bool		shared_colour_map=TRUE;	/* can we use the default? */
static Bool		use_rgb_colour_map=FALSE;/* are we doing RGB things? */
static fontstruct	fonts;			/* all the fonts */
static debug_type	debugging = debug_none;	/* debugging? */

#define DEFAULT_EVENTMASK	(KeyPressMask|ExposureMask|ButtonPressMask|			ButtonReleaseMask|PointerMotionMask|ButtonMotionMask)

#define min(x,y)	((x)<(y)?(x):(y))
#define swap(x,y)	{int t; t=x; x=y; y=t;}


/*
/^#ifdef HASPROTOS
!/endif$
stat %
*/

static void ResetWindowSettings( void );
static void InitialiseFonts( int );
static void FreeFonts( void );
static void InitialiseColour( void );
static void setRWcolours( void );
static void setROcolours( void );
static void FreeColours( void );
static void On( void );
static void Off( void );
static void Invert( void );
static void InitialiseCursors( void );
static void FreeCursors( void );
static void InitialisePixmaps( void );
static void FreePixmaps( void );
static void InitialiseClipList( int );
static void AddToClipList( int , XRectangle );
static void FreeClipList( void );
static void InitialiseWinList( int );
static void AddWin( Window , int , int );
static Window DeleteWin( Window );
static int FindWin( Window , int * , int * );
static void SetWinSize( Window , int , int );
static void FreeWinList( void );
static void fatal( char * );


/* ------------------------ Starting and Stopping ---------------------- */


void EnquireSystem( int *xpixels, int *ypixels, double *xsize, double *ysize )
{
	*xpixels	= DisplayWidth(dpy, thescreen);
	*ypixels	= DisplayHeight(dpy, thescreen);
	*xsize  	= (float)*xpixels / (float)DPI_GUESS;
	*ysize  	= (float)*ypixels / (float)DPI_GUESS;
}


void UseSharedColourMap( void )
{
	shared_colour_map = TRUE;
	use_rgb_colour_map = FALSE;
}


void UseOwnColourMap( void )
{
	shared_colour_map = FALSE;
	use_rgb_colour_map = FALSE;
}


void UseRGBColourMap( void )
{
	shared_colour_map = FALSE;
	use_rgb_colour_map = TRUE;
}


void DebugImportant( void )
{
	debugging = debug_important;
}


void DebugAll( void )
{
	debugging = debug_all;
}


void InitialiseGraf( void )
{
	XGCValues		values;

	/* Open the X connection */
	if( (dpy=XOpenDisplay(NULL)) == NULL )
	{
		fatal( "Can't open connection to X-server" );
		/*NOTREACHED*/
	}

	thescreen = DefaultScreen(dpy);
	currentwin = rootwin = RootWindow( dpy, thescreen );
	planes = DefaultDepth( dpy, thescreen );
	gc = XCreateGC( dpy, currentwin, 0L, &values );

	InitialiseColour();		/* build colour map */
	InitialiseFonts(10);		/* initialise fonts */
	InitialiseCursors();		/* define some cursors */
	InitialisePixmaps();		/* define some fill patterns */
	InitialiseClipList(10);		/* initialise cliplist structure */
	InitialiseWinList(10);		/* initialise the window list */
	ResetWindowSettings();		/* set default window settings */

	fg = colourvector[(int)Black];	/* setup GC */
	bg = colourvector[(int)White];
	XSetFillStyle( dpy, gc, FillSolid );
	On();		
}


void CloseDownGraf( void )
{
	/* Fix to bug discovered by Nick Williams, zmacu19 */
	/* if the display queue still contains requests at this   */
	/* point, the freeing of the font without flushing causes */
	/* an X protocol error.                                   */

	/* clean up any remaining display requests */
	XSync( dpy, True );

	FreeColours();
	FreeFonts();
	FreeCursors();
	FreePixmaps();
	FreeClipList();
	FreeWinList();

	XCloseDisplay( dpy );
}


Display *GetDisplay( void )
{
	return dpy;
}


/* -------------------------- Windows -------------------------------- */


Window RootWin( void )
{
	return rootwin;
}


/*
 *	ResetWindowSettings:
 *
 *	Reset mask and eventmask to defaults
 */

static void ResetWindowSettings( void )
{
	wsettings.mask      = WS_MAPWIN;
	wsettings.eventmask = DEFAULT_EVENTMASK;
}


void SetEventMask( EventMask mask )
{
	wsettings.eventmask = ExposureMask;
	if( mask & MaskKeypress )
	{
		wsettings.eventmask |= KeyPressMask;
	}
	if( mask & MaskKeyrelease )
	{
		wsettings.eventmask |= KeyReleaseMask;
	}
	if( mask & MaskMouseDown )
	{
		wsettings.eventmask |= ButtonPressMask;
	}
	if( mask & MaskMouseUp )
	{
		wsettings.eventmask |= ButtonReleaseMask;
	}
	if( mask & MaskEnter )
	{
		wsettings.eventmask |= EnterWindowMask;
	}
	if( mask & MaskLeave )
	{
		wsettings.eventmask |= LeaveWindowMask;
	}
	if( mask & MaskButtonMotion )
	{
		wsettings.eventmask |= ButtonMotionMask;
		wsettings.eventmask &= ~ PointerMotionMask;
	}
	if( mask & MaskAllMotion )
	{
		wsettings.eventmask |= ButtonMotionMask|PointerMotionMask;
	}
	if( mask & MaskResize )
	{
		wsettings.eventmask |= StructureNotifyMask;
	}
}


void SetWBackground( Colour col )
{
	wsettings.mask |= WS_BGCOL;
	wsettings.bgcol = col;
}


void SetWMinSize( int w, int h )
{
	wsettings.mask |= WS_MIN;
	wsettings.minw = w;
	wsettings.minh = h;
}


void SetWMaxSize( int w, int h )
{
	wsettings.mask |= WS_MAX;
	wsettings.maxw = w;
	wsettings.maxh = h;
}


void SetWBackingStore( void )
{
	wsettings.mask |= WS_BACKINGSTORE;
}


void SetWSaveUnder( void )
{
	wsettings.mask |= WS_SAVEUNDER;
}


void SetWNotShown( void )
{
	wsettings.mask &= ~WS_MAPWIN;
}


Window MakeWindow( Window parent, char *title, int winX, int winY, int winW, int winH )
{
	unsigned int		xswamask;
	XSetWindowAttributes	xswa;
	XSizeHints		size_h;

	/* set up the Window Attributes structure & colourmaps */
	xswa.background_pixel	= bg = colourvector[(int)White];
	xswa.border_pixel	= fg = colourvector[(int)Black];
	xswamask		= CWBackPixel | CWBorderPixel;

	if( ! shared_colour_map )
	{
		xswa.colormap	= cmap;
		xswamask	|= CWColormap;
		if( debugging == debug_all )
		{
			fprintf(stderr, "MakeWindow: own colour map desired\n");
		}
	}

	if( wsettings.mask & WS_BACKINGSTORE )
	{
		xswamask	   |= CWBackingStore;
		xswa.backing_store = WhenMapped;
		if( debugging == debug_all )
		{
			fprintf( stderr, "MakeWindow: backing store desired\n");
		}
	}
	if( wsettings.mask & WS_SAVEUNDER )
	{
		xswamask	   |= CWSaveUnder;
		xswa.save_under	   = True;
		if( debugging == debug_all )
		{
			fprintf( stderr, "MakeWindow: saveunder desired\n" );
		}
	}

	/* here we go - the window itself */
	currentwin = XCreateWindow( dpy, parent,
		winX, winY, winW, winH,
		2, CopyFromParent, CopyFromParent,
		CopyFromParent, xswamask, &xswa );

	if( ! shared_colour_map )
	{
		/* Tell X to go with this colour map */
		XSetWindowColormap( dpy, currentwin, cmap );
	}

	XSelectInput( dpy, currentwin, wsettings.eventmask );

	/* set the window name to the requested name */
	XChangeProperty( dpy, currentwin, XA_WM_NAME, XA_STRING, 8,
		PropModeReplace, title, strlen(title) );

	/*
	 *	Now tell the window manager about the size and location
	 *	we want for our windows.
	 */

	size_h.flags    = USSize;
	size_h.flags    |= (winX == -1 || winY == -1) ? PPosition : USPosition;
	size_h.x	= winX;
	size_h.y	= winY;
	size_h.width	= winW;
	size_h.height	= winH;

	if( debugging == debug_all )
	{
		fprintf( stderr, "MakeWindow: creating window width (%d,%d)\n",
			winW, winH );
		fprintf( stderr, "MakeWindow: window position is %s (%d,%d)\n",
			(winX == -1 || winY == -1) ? "PPosition" : "USPosition",
			winX, winY );
	}

	if( wsettings.mask & WS_MIN )
	{
		size_h.flags	   |= PMinSize;
		size_h.min_width   = wsettings.minw;
		size_h.min_height  = wsettings.minh;
		if( debugging == debug_all )
		{
			fprintf( stderr,
				"MakeWindow: min geom (%d,%d) desired\n",
				wsettings.minw, wsettings.minh );
		}
	}
	if( wsettings.mask & WS_MAX )
	{
		size_h.flags	   |= PMaxSize;
		size_h.max_width   = wsettings.maxw;
		size_h.max_height  = wsettings.maxh;
		if( debugging == debug_all )
		{
			fprintf( stderr,
				"MakeWindow: max geom (%d,%d) desired\n",
				wsettings.maxw, wsettings.maxh );
		}
	}

	XSetWMNormalHints( dpy, currentwin, &size_h );

	if( wsettings.mask & WS_MAPWIN )
	{
		/* try to make the window appear */
		XMapWindow( dpy, currentwin );
		if( debugging == debug_all )
		{
			fprintf( stderr, "MakeWindow: map window desired\n" );
		}
	} else if( debugging == debug_all )
	{
		fprintf( stderr, "MakeWindow: map window not desired\n" );
	}

	if( wsettings.mask & WS_BGCOL )
	{
		XSetWindowBackground( dpy, currentwin, wsettings.bgcol );
		if( debugging == debug_all )
		{
			fprintf( stderr,
				"MakeWindow: background colour = %d\n",
				wsettings.bgcol );
		}
	}

	ResetWindowSettings();	/* for next call to MakeWindow */

	AddWin( currentwin, winW, winH );

	return currentwin;
}


void SelectWindow( Window w )
{
	currentwin = w;
}


void ShowWindow( void )
{
	XMapWindow( dpy, currentwin );
}


void HideWindow( void )
{
	XUnmapWindow( dpy, currentwin );
}


void MoveResizeWindow( int x, int y, int w, int h )
{
	XMoveResizeWindow( dpy, currentwin, x, y, w, h );
}


void MoveWindow( int x, int y )
{
	XMoveWindow( dpy, currentwin, x, y );
}


void ResizeWindow( int w, int h )
{
	XResizeWindow( dpy, currentwin, w, h );
}


void CloseWindow( void )
{
	XDestroyWindow( dpy, currentwin );
	currentwin = DeleteWin( currentwin );
}


void ClearWindow( void )
{
	XClearWindow( dpy, currentwin );
}


/* --------------------------- Fonts -------------------------------- */


static void InitialiseFonts( int numfonts )
{
	fonts.size = numfonts;
	fonts.font = (XFontStruct **) malloc( numfonts * sizeof(XFontStruct *));
	fonts.name = (char **) malloc( numfonts * sizeof(char *));
	fonts.name[0] = strdup( FONT );
	if( (fonts.font[0] = XLoadQueryFont(dpy, FONT))==NULL )
	{
		char mesg[200];
		sprintf( mesg, "Display doesn't know default font '%s'", FONT );
		fatal( mesg );
		/*NOTREACHED*/
	}
	fonts.used = 1;

	SelectFont( 0 );
}


Font LoadFont( char *name )
{
	XFontStruct *f;

	if( fonts.size == fonts.used )
	{
		fonts.size += 5;
		fonts.font = (XFontStruct **) realloc( (char *) fonts.font,
				fonts.size * sizeof(XFontStruct *));
		fonts.name = (char **) realloc( (char *) fonts.name,
				fonts.size * sizeof(char *));
	}

	f = XLoadQueryFont(dpy, name);
	if( debugging == debug_all )
	{
		fprintf( stderr,
			 "LoadFont: loaded font '%s', XFontStruct * = %p\n",
			 name, (void *) f );
	}
	if( f != NULL )
	{
		fonts.font[fonts.used] = f;
		fonts.name[fonts.used] = strdup(name);
		return fonts.used++;
	}
	if( debugging != debug_none )
	{
		fprintf( stderr, "LoadFont: unknown font '%s'\n", name );
	}

	return 0;
}


Font DefaultFont( void )
{
	return 0;
}


void SelectFont( Font f )
{
	XSetFont( dpy, gc, fonts.font[f]->fid );
	fonts.current = f;
}


void GetFontName( Font f, char *name )
{
	strcpy( name, fonts.name[f] );
}


void CharSize( char ch, int *w, int *h )
{
	char		mesg[2];
	XFontStruct	*f = fonts.font[fonts.current];

	mesg[0] = ch;
	mesg[1] = '\0';

	*h = f->max_bounds.ascent + f->max_bounds.descent;
	*w = XTextWidth( f, mesg, 1 );
}


int StringWidth( char *mesg )
{
	XFontStruct	*f = fonts.font[fonts.current];

	return XTextWidth( f, mesg, strlen(mesg) );
}


int FontHeight( void )
{
	XFontStruct	*f = fonts.font[fonts.current];

	return f->max_bounds.ascent + f->max_bounds.descent;
}


void StringSize( char *mesg, int *w, int *h )
{
	*h = FontHeight();
	*w = StringWidth( mesg );
}


static void FreeFonts( void )
{
	int i;

	for( i = 0; i < fonts.used; i++ )
	{
		XFreeFont( dpy, fonts.font[i] );
		free( fonts.name[i] );
	}
	free( fonts.name );
	free( fonts.font );
}


/* --------------------------- Colours -------------------------------- */


/*
 *	InitialiseColour either sets up a new colour map, or uses
 *	the default one.
 *	Then it allocates as many of the named colours as will fit.
 */

static void InitialiseColour( void )
{
	if( use_rgb_colour_map && planes == 1 )
	{
		(void) fprintf( stderr,
		"InitialiseGraf: No RGB colour map on a monochrome screen!\n" );
		use_rgb_colour_map = FALSE;
	}

	/* count the colours in the table */
	for( tablesize=0; colour_names[tablesize] != NULL; ++tablesize );

	if( shared_colour_map )
	{
		cmap = DefaultColormap( dpy, thescreen );

		/* find the maximum number of colours available in
		 * the default colour map
		 */
		maxcolours = DisplayCells( dpy, thescreen );

	} else
	{
		cmap = XCreateColormap( dpy, rootwin,
			DefaultVisual(dpy, thescreen),
			use_rgb_colour_map ? AllocAll : AllocNone );

		/* find the maximum possible number of colours
		 * on this screen.
		 */

		maxcolours = 1<<planes;
	}
	ncolours = min( maxcolours, tablesize );

	if( debugging == debug_all )
	{
		(void) fprintf( stderr, "tablesize  = %d\n", tablesize );
		(void) fprintf( stderr, "maxcolours = %d\n", maxcolours );
		(void) fprintf( stderr, "ncolours   = %d\n", ncolours );
	}

	/* now allocate the colour vector */
	colourvector = (unsigned long *)
		malloc( (unsigned) (maxcolours * sizeof(unsigned long)) );

	/* allocate as many as poss of the named colours to pixel values
	 * mapped through the colour map
	 */

	if( use_rgb_colour_map )
	{
		setRWcolours();
	} else
	{
		setROcolours();
	}
}


static void setRWcolours( void )
{
	int i;
	XColor dummy, hw;

	for( i=0; i<maxcolours; ++i )
	{
		colourvector[i] = i;
	}

	for( i=0; i<ncolours; ++i )
	{
		if( ! XLookupColor( dpy, cmap, colour_names[i], &dummy, &hw ))
		{
			if( debugging != debug_none )
			{
				(void) fprintf( stderr,
				  "lookup for colour '%s' failed - using %s\n",
				  colour_names[i], "black" );
			}
			hw.red = hw.green = hw.blue = 0;
		}
		hw.flags = DoRed | DoGreen | DoBlue;
		hw.pixel = colourvector[i];
		XStoreColor( dpy, cmap, &hw );
	}
}


static void setROcolours( void )
{
	int i;
	XColor hw, rgb;

	for( i=0; i<ncolours; ++i )
	{
		if( XAllocNamedColor( dpy, cmap, colour_names[i], &hw, &rgb ) )
		{
			colourvector[i] = hw.pixel;
			if( debugging == debug_all )
			{
				(void) fprintf( stderr,
				   "Logical colour %d is physical pixel %ld\n",
				   i, colourvector[i] );
			}
		} else
		{
			if( debugging != debug_none )
			{
				(void) fprintf( stderr,
					"Colour allocation failed at %s\n",
					colour_names[i] );
			}
			ncolours = i;
		}
	}

	if( debugging == debug_all )
	{
		(void) fprintf( stderr, "final ncolours = %d\n", ncolours );
	}
}


void SetRGB( Colour col, int r, int g, int b )
{
	XColor rgb;

	if( ! use_rgb_colour_map )
	{
		fatal( "You can only set RGB with an RGB colour map!\n" );
		/*NOTREACHED*/
	}

	rgb.red   = r;
	rgb.green = g;
	rgb.blue  = b;
	rgb.flags = DoRed | DoGreen | DoBlue;
	rgb.pixel = colourvector[(int)col];

	XStoreColor( dpy, cmap, &rgb );
}


void GetRGB( Colour col, int *r, int *g, int *b )
{
	XColor rgb;

	if( ! use_rgb_colour_map )
	{
		fatal( "You can only get RGB from an RGB colour map!\n" );
		/*NOTREACHED*/
	}

	rgb.pixel = colourvector[(int)col];

	XQueryColor( dpy, cmap, &rgb );
	*r = rgb.red;
	*g = rgb.green;
	*b = rgb.blue;
}


void SetColour( Colour colour )
{
	if( (int)colour < 0 )
	{
		char mesg[100];
		sprintf( mesg, "SetColour: bad colour number %d", (int)colour );
		fatal( mesg );
		/*NOTREACHED*/
	}
	if( (int)colour >= ncolours && ! use_rgb_colour_map )
	{
		if( debugging != debug_none )
		{
		(void) fprintf( stderr,
			"Attempt to set colour %d - only %d colours!\n",
			(int)colour, ncolours );
		}
		colour = (Colour) (ncolours-1);
	}
	XSetForeground( dpy, gc, fg = colourvector[(int)colour] );
}


void SetBackColour( Colour colour )
{
	if( (int)colour < 0 )
	{
		char mesg[100];
		sprintf( mesg, "SetBackColour: bad colour number %d",
			 (int)colour );
		fatal( mesg );
		/*NOTREACHED*/
	}
	if( (int)colour >= ncolours && ! use_rgb_colour_map )
	{
		if( debugging != debug_none )
		{
	(void) fprintf( stderr,
		"Attempt to set back colour %d - only %d colours!\n",
		(int)colour, ncolours );
		}
		colour = (Colour) (ncolours-1);
	}
	XSetBackground( dpy, gc, bg = colourvector[(int)colour] );
	XSetWindowBackground( dpy, currentwin, bg );
}


void GetColourName( Colour colour, char *name )
{
	if( (int)colour < 0 )
	{
		char mesg[100];
		sprintf( mesg, "GetColourName: bad colour name %d",
			 (int)colour );
		fatal( mesg );
		/*NOTREACHED*/
	}
	if( (int)colour >= ncolours ) colour = (Colour) (ncolours-1);
	strcpy( name, colour_names[(int)colour] );
}


Colour FirstColour( void )
{
	return (Colour) 0;
}


Colour LastColour( void )
{
	return (Colour) (use_rgb_colour_map ? maxcolours-1 : ncolours-1);
}


int NumColours( void )
{
	return use_rgb_colour_map ? maxcolours : ncolours;
}


static void FreeColours( void )
{
	/* destroy colourmap if we are using our own */
	if( ! shared_colour_map )
	{
		XFreeColormap( dpy, cmap );
	}
	free( (void *)colourvector );
}


/* -------------------------- Events -------------------------------- */


static BOOL allownoevent = FALSE;


void AllowNoEvent( void )
{
	allownoevent = TRUE;
}


void GetEvent( Window *win, EventType *event, EventData *data )
{
	XEvent		xev;
	EventData	useifdatanull;

	/* Enhancement: These could result in null pointer dereferences */
	/* make sure that they don't 					*/
	if( event == NULL )
	{
		fatal( "Can't use GetEvent without an event" );
		/*NOTREACHED*/
	}
	*event = NoEvent;

	if( data == NULL )
	{
		/* a warning in case they didn't realise */
		if( debugging == debug_all )
		{
			(void) fprintf( stderr,
				"data parameter to GetEvent is NULL\n");
		}
		data = &useifdatanull;
	}
	data->x 	= 0;
	data->y 	= 0;
	data->ch	= '\0';
	data->button	= 0;

	do
	{
		XNextEvent(dpy, &xev);

		*win = xev.xany.window;

		switch ( xev.type )
		{

		/*
		 * EXPOSE EVENT
		 */
		case Expose:
		{
			XRectangle	r;
			XExposeEvent	*xexp = &xev.xexpose;

			/* debugging code */
			if( debugging == debug_all )
			{
				(void) fprintf( stderr,
				 "Event was an Expose: win = %ld, count = %d\n",
					xexp->window, xexp->count );
			}

			r.x	 = xexp->x;
			r.y	 = xexp->y;
			r.width	 = xexp->width;
			r.height = xexp->height;

			AddToClipList( xexp->count, r );

			if( xexp->count == 0 )
			{
				*event = EventExpose;
			}
		}
		break;
	
		/*
		 * BUTTONRELEASE and BUTTONPRESS
		 */
		case ButtonRelease:
		case ButtonPress:
		{
			XButtonEvent *xevb = &xev.xbutton;

			data->x = xevb->x;
			data->y = xevb->y;
			switch( xevb->button )
			{
			case Button1:	data->button = 1; break;
			case Button2:	data->button = 2; break;
			default:	data->button = 3; break;
			}
			*event = xevb->type == ButtonPress ?
				EventMouseDown : EventMouseUp;

			/* debugging code */
			if( debugging == debug_all )
			{
				(void) fprintf( stderr,
				 "Event was a %s: on button %d at (%d,%d)\n",
				xevb->type == ButtonPress?"ButtonPress":
						"ButtonRelease",
				data->button, xevb->x, xevb->y );
			}

		}
		break;

		/*
		 * MOTIONNOTIFY
		 */
		case MotionNotify:
		{
			XMotionEvent *xevm = &xev.xmotion;

			data->x = xevm->x;
			data->y = xevm->y;
			*event = EventMove;

			/* debugging code */
			if( debugging == debug_all )
			{
				(void) fprintf( stderr,
				"Event was a Motion: to (%d,%d)\n", xevm->x,
					 xevm->y );
			}

		}
		break;

		/*
		 * KEYPRESS/KEYRELEASE
		 */
		case KeyPress:
		case KeyRelease:
		{
			XKeyEvent	*xevk = &xev.xkey;
			KeySym		keysym;
			XComposeStatus	eek;
			int		ret;
			char		buffer[30];

			ret = XLookupString( xevk, buffer, 29, &keysym, &eek );
			if( debugging == debug_all )
			{
				(void) fprintf( stderr,
				"Event was a %s: keysym is %lu, state is %u\n",
					xevk->type == KeyPress?"KeyPress":
						"KeyRelease",
					keysym, xevk->state );
			}
			if( ret == 1 )
			{
				if( debugging == debug_all )
				{
					(void) fprintf( stderr,
					  "%s: Key is '%c' (%ld)\n",
					  xevk->type == KeyPress?"KeyPress":
							"KeyRelease",
						(char)keysym, keysym );
				}

				data->ch = keysym;
			} else if( debugging == debug_all )
			{
				(void) fprintf( stderr,
					"%s: keysym only\n",
					xevk->type == KeyPress?"KeyPress":
							"KeyRelease");
				data->ch = '\0';
			}
			data->keycode	= keysym;
			data->keystate	= xevk->state;
			*event = xevk->type == KeyPress ?
				EventKeypress : EventKeyrelease;
		}
		break;

		/*
		 * CONFIGURENOTIFY
		 */
		case ConfigureNotify:
		{
			XConfigureEvent *xevc = &xev.xconfigure;
			int pos;
			int oldw, oldh;

			pos = FindWin( xevc->window, &oldw, &oldh );
			if( oldw != xevc->width || oldh != xevc->height )
			{
				data->w = xevc->width;
				data->h = xevc->height;
				*event  = EventResize;
				SetWinSize( xevc->window, xevc->width,
					    xevc->height );
				/* debugging code */
				if( debugging == debug_all )
				{
					(void) fprintf( stderr,
						"Event was a Resize: (%d,%d)\n",
						data->w, data->h );
				}
			} else
			{
				/* debugging code */
				if( debugging == debug_all )
				{
					(void) fprintf( stderr,
						"Event was a Configure: %s\n",
						"discarded (no size change)" );
				}
			}


		}
		break;

		}
	}
	while( !allownoevent && *event == NoEvent );
}


int EventPending( void )
{
	return XPending(dpy) == 0 ? FALSE : TRUE;
}


/* -------------------------- Plot Attributes ---------------------------- */


void SetPlotAttribute( PlotAttribute p )
{
	if( current_pen != p )
	{
		switch( p )
		{
		case OnPixel:		On(); break;
		case OffPixel:		Off(); break;
		case InvertPixel:	Invert(); break;
		default:
			fatal( "SetPlotAttribute: invalid value of P" );
			/*NOTREACHED*/
		}
	}
}


static void On( void )
{
	XGCValues	xgcv;

	current_pen = OnPixel;
	XSetPlaneMask( dpy, gc, AllPlanes );
	xgcv.function = GXcopy;
	XChangeGC( dpy, gc, GCFunction, &xgcv );
	XSetForeground( dpy, gc, fg );
	XSetBackground( dpy, gc, bg );
}


static void Off( void )
{
	XGCValues	xgcv;

	current_pen = OffPixel;
	XSetPlaneMask( dpy, gc, AllPlanes );
	xgcv.function = GXcopy;
	XChangeGC( dpy, gc, GCFunction, &xgcv );
	XSetForeground( dpy, gc, bg );
	XSetBackground( dpy, gc, fg );
}


static void Invert( void )
{
	XGCValues	xgcv;

	current_pen = InvertPixel;
	XSetPlaneMask( dpy, gc, 1L );
	xgcv.function = GXinvert;
	XChangeGC( dpy, gc, GCFunction, &xgcv );
	XSetForeground( dpy, gc, fg );
	XSetBackground( dpy, gc, bg );
}


/* ---------------------- Line Thickness and Style ------------------------ */


void SetLineThickness( int w )
{
	XGCValues xgcv;
	xgcv.line_width = w;
	XChangeGC( dpy, gc, GCLineWidth, &xgcv );
}


void SetLineStyle( LineStyle s )
{
	XGCValues xgcv;
	switch( s )
	{
	case LineSolidStyle:		xgcv.line_style = LineSolid; break;
	case LineDashStyle:		xgcv.line_style = LineOnOffDash; break;
	case LineDoubleDashStyle:	xgcv.line_style = LineDoubleDash; break;
	default:
		fatal( "SetLineStyle: invalid linestyle" );
		/*NOTREACHED*/
	}
	XChangeGC( dpy, gc, GCLineStyle, &xgcv );
}


void ResetLineThicknessAndStyle( void )
{
	XGCValues xgcv;
	xgcv.line_width = 0;
	xgcv.line_style = LineSolid;
	XChangeGC( dpy, gc, GCLineWidth|GCLineStyle, &xgcv );
}


/* -------------------------- Filling Styles ---------------------------- */


void SetFillStyle( FillingStyle style )
{
	if( style == SolidStyle )
	{
		XSetFillStyle( dpy, gc, FillSolid );
	} else
	{
		XSetFillStyle( dpy, gc, FillOpaqueStippled );
		XSetStipple( dpy, gc, pattern[(int)style-1].pixmap );
	}
}


FillingStyle FirstStyle( void )
{
	return SolidStyle;
}


FillingStyle LastStyle( void )
{
	return (FillingStyle) (NUM_PATS-1);
}


/* -------------------------- Draw Operations ---------------------------- */


void DrawPoint( int x, int y )
{
	XDrawPoint( dpy, currentwin, gc, x, y );
}


void DrawLine( int x1, int y1, int x2, int y2 )
{
	XDrawLine( dpy, currentwin, gc, x1, y1, x2, y2 );
}


void DrawRectangle( int x1, int y1, int x2, int y2 )
{
	/* draw the rectangle */
	if( x1 > x2 ) swap(x1,x2);
	if( y1 > y2 ) swap(y1,y2);

	XDrawLine( dpy, currentwin, gc, x1, y1, x2, y1 );
	XDrawLine( dpy, currentwin, gc, x2, y1, x2, y2 );
	XDrawLine( dpy, currentwin, gc, x2, y2, x1, y2 );
	XDrawLine( dpy, currentwin, gc, x1, y2, x1, y1 );
}


void FillRectangle( int x1, int y1, int x2, int y2 )
{
	/* fill the rectangle */
	if( x1 > x2 ) swap(x1,x2);
	if( y1 > y2 ) swap(y1,y2);

	XFillRectangle( dpy, currentwin, gc, x1, y1, 1+x2-x1, 1+y2-y1 );
}


void DrawChar( int x, int y, char ch )
{
	char mesg[2];

	mesg[0] = ch;
	mesg[1] = '\0';

	XDrawString( dpy, currentwin, gc, x, y, mesg, 1 );
}


void DrawString( int x, int y, char *mesg )
{
	/* display the string itself */
	XDrawString( dpy, currentwin, gc, x, y, mesg, strlen(mesg) );
}


void DrawArc( int x, int y, int radius, int sangle, int eangle )
{
	int topx, topy, w, h, path;

	sangle <<= 6;	/* X11 uses 64's of a degree */
	eangle <<= 6;
	path     = 63 + eangle - sangle;

	topx  = x - radius;
	topy  = y - radius;
	h = w = 2 * radius;

	XDrawArc( dpy, currentwin, gc, topx, topy, w, h, sangle, path );
}


void DrawCircle( int x, int y, int radius )
{
	int topx, topy, w, h;

	topx  = x - radius;
	topy  = y - radius;
	h = w = 2 * radius;

	XDrawArc( dpy, currentwin, gc, topx, topy, w, h, 0, 360 * 64 );
}


void FillArc( int x, int y, int radius, int sangle, int eangle )
{
	int topx, topy, w, h, path;

	sangle <<= 6;	/* X11 uses 64's of a degree */
	eangle <<= 6;
	path     = 63 + eangle - sangle;

	topx  = x - radius;
	topy  = y - radius;
	h = w = 2 * radius;

	XFillArc( dpy, currentwin, gc, topx, topy, w, h, sangle, path );
}


void FillCircle( int x, int y, int radius )
{
	int topx, topy, w, h;

	topx  = x - radius;
	topy  = y - radius;
	h = w = 2 * radius;

	XFillArc( dpy, currentwin, gc, topx, topy, w, h, 0, 360 * 64 );
}


/* --------------------------- Cursors -------------------------------- */


static void InitialiseCursors( void )
{
	/* create some cursors */
	user_cursor = XCreateFontCursor( dpy, XC_crosshair );
	busy_cursor = XCreateFontCursor( dpy, XC_watch );
}


static void FreeCursors( void )
{
	/* zap the cursor bitmaps */
	XFreeCursor( dpy, user_cursor );
	XFreeCursor( dpy, busy_cursor );
}


void OnCursor( void )
{
	XDefineCursor( dpy, currentwin, user_cursor );
}


void BusyCursor( void )
{
	XDefineCursor( dpy, currentwin, busy_cursor );
}


void OffCursor( void )
{
	XUndefineCursor( dpy, currentwin );
}


/* --------------------------- Pixmaps -------------------------------- */


static void InitialisePixmaps( void )
{
	int i;

	/* create some pixmaps for later on */
	for( i = 0; i < NUM_PATS; i++ ) {
		pattern[i].pixmap = XCreatePixmapFromBitmapData( dpy, rootwin,
			pattern[i].data, pattern[i].width, pattern[i].height,
			0, 1, 1 );
	}
}


static void FreePixmaps( void )
{
	int i;

	for( i = 0; i < NUM_PATS; i++ )
	{
		XFreePixmap( dpy, pattern[i].pixmap );
	}
}


/* -------------------------- Clipping List -------------------------------- */
/* ---------------- for efficient redraws on expose events ----------------- */


typedef struct
{
	XRectangle *	data;
	int		size, used;
	BOOL		newseq;
} cliplist;


static cliplist clist;


/*
 *	InitialiseClipList:
 *
 *	Set the clip list system going.
 */

static void InitialiseClipList( int maxsize )
{
	clist.size	= maxsize;
	clist.data	= (XRectangle *) malloc( clist.size *
						 sizeof(XRectangle) );
	clist.used	= 0;
	clist.newseq	= TRUE;
}


/*
 *	AddToClipList:
 *
 *	Add the given rectangle (r) to the clip list
 *	(starting a new clip list if necessary).
 *
 *	(Called for every underlying X Expose event)
 */

static void AddToClipList( int count, XRectangle r )
{
	if( clist.newseq )
	{
		if( count > clist.size )
		{
			clist.size = count + 5;;
			clist.data = (XRectangle *) realloc(
					(char *) clist.data,
					clist.size * sizeof(XRectangle) );
		}
		clist.used   = 0;
		clist.newseq = FALSE;
	}
	clist.data[ clist.used++ ] = r;

	if( count == 0 )
	{
		clist.newseq = TRUE;
		/* For next time! */
	}
}


void SetClipList( void )
{
	XSetClipRectangles( dpy, gc, 0, 0, clist.data, clist.used, Unsorted );
}


void ClearClipList( void )
{
	XSetClipMask( dpy, gc, None );
}


/*
 *	FreeClipList:
 *
 *	Discard the clip list data.
 */

static void FreeClipList( void )
{
	free( (char *) clist.data );
}


/* ------------------------- The Window List ---------------------- */


static void InitialiseWinList( int n )
{
	winsize.used = 0;
	winsize.size = n;
	winsize.data = (windowstruct *) malloc( winsize.size *
						 sizeof(windowstruct) );
}


static void AddWin( Window win, int w, int h )
{
	windowstruct *wp;

	if( winsize.used == winsize.size )
	{
		winsize.size += 5;
		winsize.data = (windowstruct *) realloc(
				(char *) winsize.data,
				winsize.size * sizeof(windowstruct) );
	}
	wp        = &winsize.data[winsize.used];
	wp->win   = win;
	wp->w     = w;
	wp->h     = h;
	winsize.used++;
}


static Window DeleteWin( Window win )
{
	int pos = FindWin( win, NULL, NULL );
	int i;

	for( i = pos+1; i < winsize.size; i++ )
	{
		winsize.data[i-1] = winsize.data[i];
	}
	winsize.used--;

	return (winsize.used == 0) ? rootwin : winsize.data[0].win;
}


static int FindWin( Window win, int *wp, int *hp )
{
	windowstruct	*ptr;
	windowstruct	*beyond;

	beyond = winsize.data + winsize.used;
	for( ptr = winsize.data; ptr < beyond; ptr++ )
	{
		if( win == ptr->win )
		{
			if( wp != NULL ) *wp = ptr->w;
			if( hp != NULL ) *hp = ptr->h;
			return ptr - winsize.data;
		}
	}
	fatal( "FindWin: window not found!\n" );
	/*NOTREACHED*/
}


static void SetWinSize( Window win, int w, int h )
{
	int pos = FindWin( win, NULL, NULL );

	winsize.data[pos].w = w;
	winsize.data[pos].h = h;
}


static void FreeWinList( void )
{
	free( (char *) winsize.data );
}


/* -------------------------- Miscellaneous ------------------------------ */


void FlushOutput( void )
{
	XFlush( dpy );
}


/*
 *	Fatal:
 *
 *	Announce a fatal error and die.
 */

static void fatal( char *desc )
{
	perror( "Xgraf: Graphics Library for X11" );
	(void) fprintf( stderr, "A fatal error has occurred\n");
	(void) fprintf( stderr, "Fault is: %s\n", desc );
	exit(1);
	/*NOTREACHED*/
}


