
CC := gcc
CFLAGS := -Wall -Wextra -pedantic -g

TARGET := semafory_main

.PHONY: all clean all_ipc

all: $(TARGET) p1 p2 p3

all_ipc: klient serwer prod kons mainp

$(TARGET): semafory_main.c
	$(CC) $(CFLAGS) semafory_main.c -o $(TARGET)

p1: p1.c
	$(CC) $(CFLAGS) $< -o $@

p2: p2.c
	$(CC) $(CFLAGS) $< -o $@

p3: p3.c
	$(CC) $(CFLAGS) $< -o $@

klient: klient.c
	$(CC) $(CFLAGS) $< -o $@

serwer: serwer.c
	$(CC) $(CFLAGS) $< -o $@

prod: prod.c
	$(CC) $(CFLAGS) $< -o $@

kons: kons.c
	$(CC) $(CFLAGS) $< -o $@

mainp: mainp.c
	$(CC) $(CFLAGS) $< -o $@

prod_kons_pipe: prod_kons_pipe.c
	$(CC) $(CFLAGS) $< -o $@

producent: producent.c
	$(CC) $(CFLAGS) $< -o $@

konsument: konsument.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET) p1 p2 p3 klient serwer prod kons mainp prod_kons_pipe producent konsument wynik.txt we_*.txt wy_*.txt
