CC = cc
CFLAGS = -Wall -Wpedantic -std=gnu99 -O2 -lfcgi
TARGET = relay-control
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): src/$(TARGET).c src/$(TARGET).h
	$(CC) $(CFLAGS) -o $(TARGET) src/$(TARGET).c
	strip $(TARGET)

clean:
	$(RM) $(TARGET)

install: $(TARGET) www/index.html
	install -d $(PREFIX)/bin
	install -m 755 relay-control $(PREFIX)/bin/relay-control
	install -m 644 www/index.html /var/www/html/index.html
