sock_srv_i_cc : sock_srv_i_cc.o main.o sock_cli_i_sc.o
	gcc -o sock_srv_i_cc sock_srv_i_cc.o
	gcc -o bash main.o
	gcc -o sock_cli_i_sc sock_cli_i_sc.o

sock_srv_i_cc.o : sock_srv_i_cc.c main.c
	gcc -c sock_srv_i_cc.c

sock_cli_i_sc.o : sock_cli_i_sc.c
	gcc -c sock_cli_i_sc.c

main.o : main.c
	gcc -c main.c

clean:
	rm bash sock_cli_i_sc sock_srv_i_cc *.o