CC = gcc
bash : bash.o
	$(CC) -o bash bash.o
	rm  *.o
bash.o : bash.c
	$(CC) -c bash.c -w

clean:
	rm bash