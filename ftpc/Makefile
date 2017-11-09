CC=gcc
TARGET=ftpc
PREFIX = /usr/local

CFLAGS=-c `pkg-config fuse --cflags` -I$(PREFIX)/include -fPIC -Wall -Wextra -g
LDFLAGS= -I$(PREFIX)/lib -lsqlite3 -lftfs -g


.PHONY: all clean install uninstall

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
       
$(TARGET): ftpc.o main.o  
	$(CC) $(LDFLAGS) *.o -o $@

ftpc.o: ftpc.c $(PREFIX)/include/ftfs.h ftpc.h
	$(CC) $(CFLAGS) ftpc.c
    
main.o: main.c $(PREFIX)/include/ftfs.h ftpc.h msg-err.res msg-msg.res
	$(CC) $(CFLAGS) main.c

install: all
	install ./$(TARGET) $(PREFIX)/bin
	chmod +s $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET)
