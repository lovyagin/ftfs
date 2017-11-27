CC=gcc
TARGET=ftfs
PREFIX = /usr/local

CFLAGS=-c `pkg-config fuse --cflags` -I$(PREFIX)/include -fPIC -Wall -Wextra
LDFLAGS= `pkg-config fuse --libs` -I$(PREFIX)/lib -lsqlite3 -lftfs

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS+=-g
    LDFLAGS+=-g
else
    CFLAGS+=-DNDEBUG -O2
endif

.PHONY: all clean install uninstall

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
       
$(TARGET): ftops.o main.o  
	$(CC) $(LDFLAGS) *.o -o $@

ftops.o: ftops.c $(PREFIX)/include/ftfs.h fusecfg.h ftops.h
	$(CC) $(CFLAGS) ftops.c
    
main.o: main.c $(PREFIX)/include/ftfs.h ftops.h fusecfg.h  msg-err.res msg-msg.res
	$(CC) $(CFLAGS) main.c

install: all
	install ./$(TARGET) $(PREFIX)/bin
	ln -fs ftfs $(PREFIX)/bin/mount.ftfs
	chown root:root $(PREFIX)/bin/mount.ftfs

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET)

remake: clean all
	

reinstall: clean install
	
