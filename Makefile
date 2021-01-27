CC = cc
CFLAGS = -Wall -Wpedantic -std=gnu99 -O2 -lfcgi
TARGET = relay-control
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
	strip $(TARGET)

clean:
	$(RM) $(TARGET)

install: $(TARGET) index.html
	install -d $(PREFIX)/bin
	install -m 755 relay-control $(PREFIX)/bin/relay-control
	install -m 644 index.html /var/www/html/index.html
