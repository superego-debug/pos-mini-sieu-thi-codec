CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

pos: main.c
	$(CC) $(CFLAGS) -o pos main.c

run: pos
	./pos

clean:
	rm -f pos

.PHONY: run clean
