CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I./include
LDFLAGS =
OBJ = src/main.o src/network.o src/user.o src/utils.o

all: circ

debug: CFLAGS += -g -fsanitize=address
debug: LDFLAGS += -fsanitize=address
debug: circ

circ: $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

run: circ
	./circ

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o circ
