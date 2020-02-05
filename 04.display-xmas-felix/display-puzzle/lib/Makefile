CC	=	gcc
X	=	/usr/X11R6
CFLAGS	=       -I. -I$(X)/include -g
LDFLAGS	=	
LIBFILE	=	lib$(ARCH)xmm.a
crap	=	*.o core a.out *.lst lib*.a
OBJS	=	mmsupport.o Xgraf.o

all:	$(LIBFILE)

clean:	
		/bin/rm -f $(crap) 

mmsupport.o:	mmsupport.h
	$(CC) -c $(CFLAGS) -Wall mmsupport.c

Xgraf.o:	Xgraf.h Xgrafprivate.h Xcolours.h
	$(CC) -c $(CFLAGS) Xgraf.c

$(LIBFILE):	$(OBJS)
	rm -f $(LIBFILE)
	ar rc $(LIBFILE) $(OBJS)
	ranlib $(LIBFILE)

mmsupport.o:	mmsupport.c mmsupport.h 
