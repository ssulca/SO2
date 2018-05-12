CC = gcc

CFLAGS = -fopenmp -Wall -Werror -pedantic
LFLAGS = -lm -fopenmp

OpenMP : main.o
	$(CC) -o openmp main.o $(LFLAGS)
	rm  *.o

main.o : main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm openmp
