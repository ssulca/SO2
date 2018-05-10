CC = gcc
# CFLAGS = -Wall -Werror -pedantic
LFLAGS = -lm

sock_srv_i_cc : main.o
	$(CC) -o openmp main.o $(LFLAGS)
	rm  *.o

main.o : main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm openmp
