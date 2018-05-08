CC = gcc
#CFLAGS = -Wall -Werror -pedantic
sock_srv_i_cc : main.o
	$(CC) -o openmp main.o
	rm  *.o
main.o : main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm openmp
