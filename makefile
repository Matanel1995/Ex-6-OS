CC = gcc
C = -c
O = -o
lp= --shared -fPIC -pthread 
Flag = -Wall -g -c

all: main1 client selectclient selectserver Singelton Guard Reactor

main1: Queue.o main1.c
	$(CC) main1.c $(lp) $(O) main1

selectserver: selectserver.c
	$(CC) selectserver.c $(lp) $(O) selectserver

selectclient: selectclient.c
	$(CC) selectclient.c $(lp) $(O) selectclient

client: client.cpp
	$(CC) client.cpp $(lp) $(O) client

Singelton: Singelton.cpp
	$(CC) Singelton.cpp $(lp) $(O) Singelton

Guard: Guard.cpp
	$(CC) Guard.cpp $(lp) $(O) Guard

Reactor: Reactor.cpp
	$(CC) Reactor.cpp $(lp) $(O) Reactor

Queue.o:  Node.o Queue.c
	$(CC) $(lp)$(Flag)  Queue.c

Node.o:	Node.c
	$(CC) $(Flag)  Node.c

.PHONY: clean all

clean:
	rm -f *.o server client selectclient selectserver Singelton Guard Reactor main1
