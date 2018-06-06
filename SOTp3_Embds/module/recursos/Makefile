CFLAGS = -Wall -Werror -pedantic
CC=gcc

all: exec_insmod exec_rmmod clean

exec_insmod: exec_insmod.o
	sudo $(CC) -o exec_insmod exec_insmod.o
	sudo chmod a+s exec_insmod

exec_rmmod: exec_rmmod.o
	sudo $(CC) -o exec_rmmod exec_rmmod.o
	sudo chmod a+s exec_rmmod

exec_rmmod.o : exec_rmmod.c
	sudo $(CC) -c exec_rmmod.c $(CFLAGS)

exec_insmod.o : exec_insmod.c
	sudo $(CC) -c exec_insmod.c $(CFLAGS)

clean:
	sudo rm  *.o