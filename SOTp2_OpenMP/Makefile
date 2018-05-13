CC = gcc

CFLAGS = -fopenmp -Wall -Werror -pedantic
LFLAGS = -lm -fopenmp
# PFLAGS = -pg -Q

OpenMP : main.o
	$(CC) -o openmp main.o $(LFLAGS) $(PFLAGS)
	rm  *.o

main.o : main.c
	$(CC) -c main.c $(CFLAGS) $(PFLAGS)

clean:
	rm openmp
