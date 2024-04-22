CFLAGS=-g -Wall -W -std=c99
LDFLAGS=-L. -Wl,-R. -lrkht -static

all: librkht.a librkht.so main

main: main.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

librkht.so: rk-ht.c
	$(CC) -fPIC -shared $^ -o $@ $(CFLAGS)

librkht.a: rk-ht.o
	$(AR) rcs $@ $^

.PHONY: clean

clean:
	rm -f *.o librkht.a librkht.so main
