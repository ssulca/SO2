CC = gcc
CFLAGS = -Wall -Werror -pedantic
sock_srv_i_cc : sock_srv_i_cc.o bash.o
	$(CC) -o server sock_srv_i_cc.o
	$(CC) -o bash bash.o
	rm  *.o
sock_srv_i_cc.o : sock_srv_i_cc.c bash.c
	$(CC) -c sock_srv_i_cc.c $(CFLAGS)

bash.o : bash.c
	$(CC) -c bash.c -w

clean:
	rm bash server