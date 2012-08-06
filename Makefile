SOURCES=$(wildcard *.c)
PROGRAM=test-rl
CC=clang
CFLAGS+=-Wall -std=c99 -D_XOPEN_SOURCE=90000

all: $(PROGRAM)

$(PROGRAM): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline -lmpdclient

clean: $(PROGRAM)
	-rm $(PROGRAM)
