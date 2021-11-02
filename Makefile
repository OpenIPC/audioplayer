CC=arm-openipc-linux-musleabi-gcc
MAJ=$(HOME)/git/majestic
CFLAGS=-I$(MAJ)/lib/liblame.hi3516ev300/include -I$(MAJ)/thirdparty/nda/hi3516ev300/include
LDFLAGS=-L$(MAJ)/lib/liblame.hi3516ev300/lib -L$(MAJ)/thirdparty/nda/hi3516ev300/lib
LDLIBS=-lmp3lame -lmpi -lsecurec -lupvqe -ldnvqe -lVoiceEngine

audioplayer: player.o errors.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	sudo cp $@ /mnt/noc/sdk
