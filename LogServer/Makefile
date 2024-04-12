CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=LogServer.cpp
LIBS=-lpthread

LogServer: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o LogServer

all: LogServer