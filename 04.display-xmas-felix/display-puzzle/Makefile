CC	=	gcc
CFLAGS  =       -Ilib -Wall -g
LDFLAGS =       -Llib -l$(ARCH)xmm -L/usr/X11R6/lib -lX11
crap	=	*.o core a.out *.lst
execs	=	mm5-5-randomsecret mm5-5-fixedsecret

all:	libs $(execs)

libs:
	cd lib; make

clean:	
	/bin/rm -f $(crap) $(execs)
	cd lib; make clean

mm5-5-randomsecret:	mm5-5-randomsecret.o lib/lib$(ARCH)xmm.a
	$(CC) -o mm5-5-randomsecret mm5-5-randomsecret.o $(LDFLAGS)

mm5-5-randomsecret.o:	mm5-5-randomsecret.c 

mm5-5-fixedsecret:	mm5-5-fixedsecret.o lib/lib$(ARCH)xmm.a
	$(CC) -o mm5-5-fixedsecret mm5-5-fixedsecret.o $(LDFLAGS)

mm5-5-fixedsecret.o:	mm5-5-fixedsecret.c 
