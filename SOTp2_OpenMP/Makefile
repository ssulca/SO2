CC = gcc
# CFLAGS = -Wall -Werror -pedantic
LFLAGS = -lm

OpenMP : main.o
	$(CC) -o openmp main.o $(LFLAGS)
	rm  *.o

main.o : main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm openmp
