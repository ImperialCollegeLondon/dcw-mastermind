/*
 *	misc.h: Some miscellaneous useful stuff
 *
 *	(C) Duncan C. White..
 */

/*
 * ALLOC and NEW: easier form of malloc.
 * eg. typedef struct list *list; struct list { int hd, list tl };
 * list l = NEW( list );
 */
#define ALLOC(n,t)     ( (t) malloc((n)*sizeof(struct t)) )
#define NEW(t)         ALLOC(1,t)

/*
 * COPYOF: make copy of old string into new:
 */
#define COPYOF(new,old)	{new=malloc(1+strlen(old));if(new)strcpy(new,old);}

/*
 * simpler form of string comparison..
 */
#define streq(x,y)	(strcmp((x),(y))==0)
#define strneq(x,y,n)	(strncmp((x),(y),(n))==0)

/*
 * miscellaneous useful macros
 */
#define flush()		fflush(stdout)
#define readln(f)	while(getc(f)!='\n')
#define min(x,y)	( (x)<(y)?(x):(y) )
#define max(x,y)	( (x)>(y)?(x):(y) )

/*
 * boolean data type..
 */

#ifndef BOOL
#define BOOL char
#define BOOLEAN char
#define TRUE    1
#define FALSE   0
#endif

/*
 * useful procedure type..
 */
typedef void (*proc)();
