all: tweeter

tweeter: main.c
	gcc -std=c99 -lpthread -lrt -o tweeter main.c
