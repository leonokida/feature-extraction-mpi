CFLAGS = -Wall

all: mpibase

mpibase: mpibase.c
	mpicc mpibase.c -o mpibase $(CFLAGS)

clean:
	-rm -f mpibase