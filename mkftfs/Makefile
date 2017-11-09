CC=gcc
TARGET=mkftfs
PREFIX = /usr/local

CFLAGS=-c `pkg-config fuse --cflags` -I$(PREFIX)/include -fPIC -Wall -Wextra -g
LDFLAGS= -I$(PREFIX)/lib -lsqlite3 -lftfs -g


.PHONY: all clean install uninstall

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
       
$(TARGET): main.o  
	$(CC) $(LDFLAGS) *.o -o $@
    
main.o: main.c $(PREFIX)/include/ftfs.h msg-err.res msg-msg.res
	$(CC) $(CFLAGS) main.c

install: all
	install ./$(TARGET) $(PREFIX)/bin
	ln -fs mkftfs $(PREFIX)/bin/mkfs.ftfs
	chown root:root $(PREFIX)/bin/mkfs.ftfs

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET)
