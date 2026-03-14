CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -I./include -I./lib
LDFLAGS =
SRCS = $(wildcard src/*.c)
OBJ = $(SRCS:.c=.o)

all: circ

debug: CFLAGS += -g -fsanitize=address
debug: LDFLAGS += -fsanitize=address
debug: circ

circ: $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o circ
