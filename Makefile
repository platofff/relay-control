CC = cc
CFLAGS = -Wall -Wpedantic -std=gnu99 -O2 -lfcgi
TARGET = relay-control
PREFIX = /usr/local
WWW_PREFIX = /var/www/html

all: $(TARGET)

$(TARGET): src/$(TARGET).c include/$(TARGET).h
	$(CC) $(CFLAGS) -Iinclude -o $(TARGET) src/$(TARGET).c
	strip $(TARGET)

clean:
	$(RM) $(TARGET)

install: $(TARGET) www/index.html www/favicon.ico
	install -d $(PREFIX)/bin
	install -d $(WWW_PREFIX)
	install -m 755 relay-control $(PREFIX)/bin/relay-control
	install -m 644 www/index.html $(WWW_PREFIX)/index.html
	install -m 644 www/favicon.ico $(WWW_PREFIX)/favicon.ico
