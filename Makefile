CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -I./include
LDFLAGS =
SRCS = $(wildcard src/*.c)
OBJ = $(SRCS:.c=.o)

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
