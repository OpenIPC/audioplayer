CC=arm-openipc-linux-musleabi-gcc
MAJ=$(HOME)/git/majestic
CFLAGS=-I$(MAJ)/liblame.hi3516ev300/include
LDFLAGS=-L$(MAJ)/liblame.hi3516ev300/lib
LDLIBS=-lmp3lame

player: player.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	sudo cp $@ /mnt/noc/sdk
