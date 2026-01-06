CC=cc
CFLAGS=-Wall -Wextra
LIBS=-lncurses

SRC=$(wildcard src/*.c)
OUT=phl

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(OUT)

