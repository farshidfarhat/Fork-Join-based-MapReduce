CC = gcc

CSIMDIR = /home/software/csim18/gcc/lib

INCLUDEDIR = $(CSIMDIR)
CSIMLIB = $(CSIMDIR)/csim.gcc.a
FILENAME = mm1
MY_DEB = no
CFLAGS = -I$(INCLUDEDIR) -g

objects = mm1.o

default : $(objects)
	$(CC) $(CFLAGS) -o $(FILENAME) $(objects) -lm -lpthread $(CSIMLIB)
MM1 : mm1.c
	$(CC) $(CFLAGS) -c mm1.c
clean:
	rm -f *~ $(FILENAME)
	rm -f *.o


