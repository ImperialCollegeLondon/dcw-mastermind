CC	=	gcc
CFLAGS  =       -Ilib -Wall -g
LDFLAGS =       -Llib -l$(ARCH)xmm -L/usr/X11R6/lib -lX11
crap	=	*.o core a.out *.lst
execs	=	mastermind

all:	libs $(execs)

libs:
	cd lib; make

clean:	
	/bin/rm -f $(crap) $(execs)
	cd lib; make clean

mastermind:	mastermind.o lib/lib$(ARCH)xmm.a
	$(CC) -o mastermind mastermind.o $(LDFLAGS)

mastermind.o:	mastermind.c 
