/*
 * Xgrafprivate.h
 *
 * Private include file for Xgraf.
 */

/* default sizes */
#define DPI_GUESS	75
#define PASCALSTRLEN	100

/* Some x-related stuff */
#define FONT		"fixed"
#define BORDER		2

/* some bit maps */

#define PAT_1_WIDTH 8
#define PAT_1_HEIGHT 2
static char pat_1[] =
{
	0x55, 0x55		/* Fine vertical lines */
};

#define PAT_2_WIDTH 8
#define PAT_2_HEIGHT 2
static char pat_2[] =
{
	0x0, 0xff		/* Fine horiz lines */
};

#define PAT_3_WIDTH 8
#define PAT_3_HEIGHT 2
static char pat_3[] =
{
	0xcc, 0xcc		/* Thick vertical lines */
};

#define PAT_4_WIDTH 8
#define PAT_4_HEIGHT 4
static char pat_4[] =
{
	0x00, 0x00, 0xff, 0xff	/* Thick horiz lines */
};

#define PAT_5_WIDTH 15
#define PAT_5_HEIGHT 3
static char pat_5[] =
{
	0x49, 0x12,		/* Fine left diagonal lines */
	0x92, 0x24,
	0x24, 0x49
};


#define PAT_6_WIDTH 15
#define PAT_6_HEIGHT 3
static char pat_6[] =
{
	0x24, 0x49,		/* Fine right diagonal lines */
	0x92, 0x24,
	0x49, 0x12
};

#define PAT_7_WIDTH 8
#define PAT_7_HEIGHT 4
static char pat_7[] =
{
	0x33, 0x66, 0xcc, 0x99	/* Thick left diagonal lines */
};

#define PAT_8_WIDTH 8
#define PAT_8_HEIGHT 4
static char pat_8[] =
{
	0x99, 0xcc, 0x66, 0x33	/* Thick right diagonal lines */
};

#define PAT_9_WIDTH 8
#define PAT_9_HEIGHT 4
static char pat_9[] =
{
	0x33, 0x33, 0xcc, 0xcc	/* Fine checked squares */
};

#define PAT_10_WIDTH 8
#define PAT_10_HEIGHT 8
static char pat_10[] =
{
	0x0f, 0x0f, 0x0f, 0x0f,	/* Thick checked squares */
	0xf0, 0xf0, 0xf0, 0xf0
};

#define PAT_11_WIDTH 8
#define PAT_11_HEIGHT 2
static char pat_11[] =
{
	0x55, 0xaa		/* Grey style - ie. superfine checks */
};

struct pat_struct
{
	Pixmap	pixmap;
	char *	data;
	int	width, height;
};

static struct pat_struct pattern[] =
{
	{ (Pixmap) NULL, pat_1 , PAT_1_WIDTH , PAT_1_HEIGHT },
	{ (Pixmap) NULL, pat_2 , PAT_2_WIDTH , PAT_2_HEIGHT },
	{ (Pixmap) NULL, pat_3 , PAT_3_WIDTH , PAT_3_HEIGHT },
	{ (Pixmap) NULL, pat_4 , PAT_4_WIDTH , PAT_4_HEIGHT },
	{ (Pixmap) NULL, pat_5 , PAT_5_WIDTH , PAT_5_HEIGHT },
	{ (Pixmap) NULL, pat_6 , PAT_6_WIDTH , PAT_6_HEIGHT },
	{ (Pixmap) NULL, pat_7 , PAT_7_WIDTH , PAT_7_HEIGHT },
	{ (Pixmap) NULL, pat_8 , PAT_8_WIDTH , PAT_8_HEIGHT },
	{ (Pixmap) NULL, pat_9 , PAT_9_WIDTH , PAT_9_HEIGHT },
	{ (Pixmap) NULL, pat_10, PAT_10_WIDTH, PAT_10_HEIGHT },
	{ (Pixmap) NULL, pat_11, PAT_11_WIDTH, PAT_11_HEIGHT },
};

#define NUM_PATS	(sizeof(pattern)/sizeof(struct pat_struct))


char *color_names[] = {
	"White",
	"Black",
	"Red",
	"Green",
	"Blue",
	"Yellow",
	"Cyan",
	"Magenta",
	"Brown",
	"Gold",
	"Grey",
	"Pink",
	"Orange",
	"Violet",
	"Maroon",
	"Salmon",
	"Aquamarine",
	"Coral",
	"Khaki",
	"Orchid",
	"Plum",
	"Sienna",
	"Tan",
	"Thistle",
	"Turquoise",
	"Wheat",
	"MediumAquamarine",
	"CadetBlue",
	"CornflowerBlue",
	"DarkSlateBlue",
	"LightBlue",
	"LightSteelBlue",
	"MediumBlue",
	"MediumSlateBlue",
	"MidnightBlue",
	"NavyBlue",
	"SkyBlue",
	"SlateBlue",
	"SteelBlue",
	"Firebrick",
	"SandyBrown",
	"Goldenrod",
	"MediumGoldenrod",
	"DarkGreen",
	"DarkOliveGreen",
	"ForestGreen",
	"LimeGreen",
	"MediumForestGreen",
	"MediumSeaGreen",
	"MediumSpringGreen",
	"PaleGreen",
	"SeaGreen",
	"SpringGreen",
	"YellowGreen",
	"DarkSlateGrey",
	"DimGrey",
	"LightGrey",
	"DarkOrchid",
	"MediumOrchid",
	"IndianRed",
	"MediumVioletRed",
	"OrangeRed",
	"VioletRed",
	"DarkTurquoise",
	"MediumTurquoise",
	"BlueViolet",
	"GreenYellow",
	NULL
};

#define colour_names color_names
