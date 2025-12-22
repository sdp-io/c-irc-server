CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I./include
OBJ = src/main.o src/network.o

circ: $(OBJ)
	$(CC) $^ -o $@

run: circ
	./circ

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o circ
