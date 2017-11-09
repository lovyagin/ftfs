CC=gcc
CFLAGS=-c `pkg-config fuse --cflags` -fPIC -Wall -Wextra -g
LDFLAGS= -lsqlite3 -shared -g

TARGET=libftfs.so
PREFIX = /usr/local

.PHONY: all clean install uninstall

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
       
$(TARGET): cityhash.o db.o ftfs.o fxattr.o misc.o msg.o xattr.o 
	$(CC) $(LDFLAGS) *.o -o $@
    
cityhash.o: cityhash.c bswap.h
	$(CC) $(CFLAGS) cityhash.c
    
db.o: db.c ftfs.h
	$(CC) $(CFLAGS) db.c
        
ftfs.o: ftfs.c ftfs.h xattr.h fxattr.h db.h
	$(CC) $(CFLAGS) ftfs.c
    
fxattr.o: fxattr.c ftfs.h fxattr.h bswap.h 
	$(CC) $(CFLAGS) fxattr.c

xattr.o: xattr.c ftfs.h xattr.h 
	$(CC) $(CFLAGS) xattr.c

misc.o: misc.c ftfs.h 
	$(CC) $(CFLAGS) misc.c

msg.o: msg.c ftfs.h msg-err.res msg-msg.res
	$(CC) $(CFLAGS) msg.c

install: all
	install ./$(TARGET) $(PREFIX)/lib
	cp -a ./ftfs.h $(PREFIX)/include/ftfs.h
	ldconfig /usr/local/lib

uninstall:
	rm -rf $(PREFIX)/lib/$(TARGET)
	rm -rf $(PREFIX)/include/ftfs.h
