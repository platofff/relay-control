// FastCGI socket path.
#define SOCKET_PATH "/run/relay-control.sock"
// Default www-data GID and UID in Debian
#define SOCKET_OWNER_UID 33
#define SOCKET_OWNER_GID 33

typedef struct {
  const char id[4];
  const char* name;
  const char* direction_path;
  const char* value_path;
} Relay;

#define GPIO_DIRECTION(gpio) "/sys/class/gpio/gpio" gpio "/direction"
#define GPIO_VALUE(gpio) "/sys/class/gpio/gpio" gpio "/value"
#define RELAY(gpio, label) {gpio, label, GPIO_DIRECTION(gpio), GPIO_VALUE(gpio)}

/*
First value is GPIO number for exporting via /sys/class/gpio/export.
In Debian on Raspbrry Pi 3B GPIO indexes have +458 offset.
For Orange Pi's please read this: https://linux-sunxi.org/GPIO

Second value is relay's name in web interface.

WARNING: You must use only PULLED-DOWN GPIO pins on your board!
*/
#define RELAYS_COUNT 5
Relay RELAYS[RELAYS_COUNT] = {
  RELAY("477", "Relay A"),
  RELAY("478", "Relay B"),
  RELAY("479", "Relay C"),
  RELAY("480", "Relay D"),
  RELAY("481", "Relay E")
};
