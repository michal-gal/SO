CC := gcc
CFLAGS := -Wall -Wextra -pedantic -g

BINARIES := fork1_3 dziecko

.PHONY: all clean

all: $(BINARIES)

fork1_3: fork1_3.c
	$(CC) $(CFLAGS) $< -o $@

dziecko: dziecko.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BINARIES)
