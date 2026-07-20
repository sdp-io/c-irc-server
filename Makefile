CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -O3 -std=gnu99 -I./include -I./lib
LDFLAGS =
SRCS = $(wildcard src/*.c)
OBJ = $(SRCS:.c=.o)

all: circ

debug: CFLAGS += -g -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: circ

circ: $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o circ
