.c.o:
	gcc -g -c $?

# compile client and server
all: client server

# compile client only 
client: 
	gcc -o SimpClient src/SimpClient.c \
	src/util.c -lnsl

#compile server only
server: 
	gcc -o SimpServer src/SimpServer.c \
	src/util.c -lnsl