CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

circ: src/main.c
	$(CC) $(CFLAGS) -o main src/main.c

run: main
	./main

clean:
	rm -f src/*.o main
