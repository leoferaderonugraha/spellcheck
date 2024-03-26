CC = g++
CFLAGS = -Wall -g

build: main

main: obj
	$(CC) $(CFLAGS) -o main main.o

obj:
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f *.o main
