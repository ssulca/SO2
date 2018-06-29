CC = gcc
CFLAGS = -Wall -Werror -pedantic
sock_cli_i_cc : sock_cli_i_cc.o
	$(CC) -o cliente sock_cli_i_cc.o
	rm  *.o
sock_cli_i_cc.o : sock_cli_i_cc.c
	$(CC) -c sock_cli_i_cc.c $(CFLAGS)

clean:
	rm  cliente