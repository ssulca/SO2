sock_cli_i_cc : sock_cli_i_cc.o sock_srv_i_sc.o
	gcc -o sock_cli_i_cc sock_cli_i_cc.o
	gcc -o sock_srv_i_sc sock_srv_i_sc.o

sock_cli_i_cc.o : sock_cli_i_cc.c
	gcc -c sock_cli_i_cc.c

sock_srv_i_sc.o : sock_srv_i_sc.c
	gcc -c sock_srv_i_sc.c

clean:
	rm sock_srv_i_sc sock_cli_i_cc *.o