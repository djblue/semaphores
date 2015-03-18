all: tweeter

tweeter: main.c
	gcc -O3 -std=c99 -lpthread -o tweeter main.c
