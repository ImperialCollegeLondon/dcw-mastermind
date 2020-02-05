CC	=	gcc
XGRAF   =       /homes/dcw/src/xgraf/lib
CFLAGS  =       -I. -I$(XGRAF) -Wall -g
LDFLAGS =       -L$(XGRAF) -lgraf -L/usr/X11R6/lib -lX11
crap	=	*.o core a.out *.aux *.dvi *.log
execs	=	mm3

all:	$(execs) Mastermind5.pdf

clean:	
	/bin/rm -f $(crap) $(execs)

mm3:	mm3.o mmsupport.o
	$(CC) -o mm3 mm3.o mmsupport.o $(LDFLAGS)

mmsupport.o:	mmsupport.c mmsupport.h 

Mastermind5.pdf:	Mastermind5.tex
	pdflatex Mastermind5
