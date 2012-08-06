SOURCES=$(wildcard *.c)
H_SOURCES=$(wildcard *.h)
PROGRAM=mpccmd
CC=clang
CFLAGS+=-Wall -std=c99 -D_XOPEN_SOURCE=90000

all: $(PROGRAM)

$(PROGRAM): $(SOURCES) | $(H_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline -lmpdclient

clean: $(PROGRAM)
	-rm $(PROGRAM)
