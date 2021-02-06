// FastCGI socket path.
#define SOCKET_PATH "/run/relay-control.sock"
// Default www-data GID and UID in Debian
#define SOCKET_OWNER_UID 33
#define SOCKET_OWNER_GID 33

typedef struct {
  const char id[4];
  const char* name;
}Relay;
/*
First value is GPIO number for exporting via /sys/class/gpio/export.
In Debian on Raspbrry Pi 3B this numbers have +458 offset.

Second value is relay's name in web interface.

WARNING: You must use only PULLED-DOWN GPIO pins on your board!
*/
#define RELAYS_COUNT 5
Relay RELAYS[RELAYS_COUNT] = {
  { "477", "Relay A" },
  { "478", "Relay B" },
  { "479", "Relay C" },
  { "480", "Relay D" },
  { "481", "Relay E" }
};

