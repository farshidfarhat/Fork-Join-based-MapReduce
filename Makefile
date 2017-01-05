CC = gcc

CSIMDIR = /home/software/csim18/gcc/lib

INCLUDEDIR = $(CSIMDIR)
CSIMLIB = $(CSIMDIR)/csim.gcc.a
FILENAME = mst
MY_DEB = no
CFLAGS = -I$(INCLUDEDIR) -g

objects = mst_v2.o

default : $(objects)
	$(CC) $(CFLAGS) -o $(FILENAME) $(objects) -lm -lpthread $(CSIMLIB)
MST : mst_v2.c
	$(CC) $(CFLAGS) -c mst_v2.c
clean:
	rm -f *~ $(FILENAME)
	rm -f *.o


