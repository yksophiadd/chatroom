all: server

server: main.c
	gcc main.c -o server -Wall

run: server
	./server

.PHONY: all run
