sock_srv_i_cc : sock_srv_i_cc.o main.o
	gcc -o sock_srv_i_cc sock_srv_i_cc.o
	gcc -o bash main.o
sock_srv_i_cc.o : sock_srv_i_cc.c main.c
	gcc -c sock_srv_i_cc.c
	gcc -c main.c
clean:
	rm bash sock_srv_i_cc *.o