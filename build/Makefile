all: logappend logread

CFLAGS=-g -Wall
LFLAGS=-lcrypto

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
CFLAGS += -DLINUX -I/usr/local/ssl/include -L/usr/local/ssl/lib
endif

logappend: logappend_skel.o
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

logread: logread_skel.o
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

logappend_skel.o: logappend_skel.c
	$(CC) -c -o $@ $(CFLAGS) $^

logread_skel.o: logread_skel.c
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f *.o logappend logread
