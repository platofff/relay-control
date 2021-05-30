#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcgiapp.h>

#include "relay-control.h"

static int socket_id;

void fail(const char* restrict format, ...) {
  fprintf(stderr, format);
  exit(1);
}

void gpio_unexport(__attribute__((unused)) const int signal) {
  int unexport_data = open("/sys/class/gpio/unexport", O_WRONLY);
  if (unexport_data == -1) {
    fputs("Unable to open /sys/class/gpio/unexport\n", stderr);
    return;
  }
  for (unsigned short i = 0; i < RELAYS_COUNT; i++)
    if (write(unexport_data, RELAYS[i].id, 3) != 3) {
      fputs("Error writing to /sys/class/gpio/unexport\n", stderr);
      close(unexport_data);
    }
  close(unexport_data);
  puts("");
  exit(0);
}

bool gpio_set_active(const char* gpio, bool active) {
  // Checking existance of provided relay
  for (unsigned short i = 0; i <= RELAYS_COUNT; i++) {
    if (i == RELAYS_COUNT) {
      fprintf(stderr, "GPIO pin %d isn't configured.\n", i);
      return 0;
    } else if (strcmp(RELAYS[i].id, gpio) == 0) {
      // Setting direction "out" or "in" for this GPIO pin
      int direction_data = open(RELAYS[i].direction_path, O_WRONLY);
      char* direction = active ? "out" : "in";
      if (direction_data == -1 || write(direction_data, direction, 3) != 3) {
        fprintf(stderr, "Error writing to %s\n", RELAYS[i].direction_path);
        close(direction_data);
        return 0;
      }
      close(direction_data);
      if (active) {
        int value_data = open(RELAYS[i].value_path, O_WRONLY);
        if (value_data == -1 || write(value_data, "1", 1) != 1) {
          fprintf(stderr, "Error writing to %s\n", RELAYS[i].value_path);
          close(value_data);
          return 0;
        }
        close(value_data);
      }
      break;
    }
  }
  return 1;
}

bool gpio_get_active(const char* direction_path) {
  char direction[4] = {0};
  int direction_data = open(direction_path, O_RDONLY);
  if (direction_data == -1 || read(direction_data, direction, 3) != 3) {
    fprintf(stderr, "Error while reading %s\n", direction_path);
    close(direction_data);
    return 0;
  }
  close(direction_data);
  if (strcmp(direction, "out") == 0)
    return 1;
  else
    return 0;
}

bool starts_with(const char* a, const char* b) {
  if (strncmp(a, b, strlen(b)) == 0)
    return 1;
  return 0;
}

// Removes symbols from beginning of the string
char* rm_syms(const char* str, unsigned char count) {
  size_t len = strlen(str);
  char* result = malloc(len - count + 1);
  for (unsigned char i = count; i < len; i++) {
    result[i - count] = str[i];
  }
  result[len - count] = '\0';
  return result;
}

void http_plain(char* status, FCGX_Stream* stream) {
  FCGX_FPrintF(stream,
               "Status: %s\r\n"
               "Content-type: text/plain\r\n\r\n",
               status);
}

int main() {
  // Export GPIO pins
  int export_data = open("/sys/class/gpio/export", O_WRONLY);
  if (export_data == -1)
    fail("Unable to open /sys/class/gpio/export\n");

  for (unsigned short i = 0; i < RELAYS_COUNT; i++)
    if (write(export_data, RELAYS[i].id, 4) != 4) {
      close(export_data);
      fail("Error writing to /sys/class/gpio/export\n");
    }
  close(export_data);
  // Unexport GPIO on Ctrl + C
  signal(SIGINT, gpio_unexport);
  // GPIO init is finished

  // FastCGI init
  if (FCGX_Init() != 0)
    fail("Failed to init FastCGI");
  socket_id = FCGX_OpenSocket(SOCKET_PATH, 20);

  // Changing owner of the socket file
  if (chown(SOCKET_PATH, SOCKET_OWNER_UID, SOCKET_OWNER_GID) == -1) {
    fprintf(stderr, "Failed to change owner of %s", SOCKET_PATH);
  }

  FCGX_Request request;
  if (FCGX_InitRequest(&request, socket_id, 0) != 0)
    fail("Cannot init request");

  // Main FastCGI loop
  while (FCGX_Accept_r(&request) >= 0) {
    char* query_string = FCGX_GetParam("QUERY_STRING", request.envp);
    char* request_method = FCGX_GetParam("REQUEST_METHOD", request.envp);
    if (strcmp(query_string, "status=csv") == 0 && strcmp(request_method, "GET") == 0) {
      FCGX_PutS(
          "Status: 200 OK\r\n"
          "Content-type: text/csv\r\n\r\n"
          "relay,active,name\n",
          request.out);
      for (unsigned short i = 0; i < RELAYS_COUNT; i++)
        FCGX_FPrintF(request.out, "%s,%d,%s\n", RELAYS[i].id,
                     gpio_get_active(RELAYS[i].direction_path), RELAYS[i].name);
      continue;
    } else if (strcmp(request_method, "POST") == 0) {
      // Get POST body data
      size_t post_len = atoi(FCGX_GetParam("CONTENT_LENGTH", request.envp));
      char* post_data = malloc(post_len + 1);
      post_data[post_len] = '\0';
      FCGX_GetStr(post_data, post_len, request.in);

      if (starts_with(post_data, "off=")) {
        char* gpio = rm_syms(post_data, 4);
        if (gpio_set_active(gpio, 0)) {
          http_plain("200 OK", request.out);
          FCGX_PutS("ok\n", request.out);
        } else {
          http_plain("500 Internal Server Error", request.out);
          FCGX_PutS("error\n", request.out);
        }
        free(post_data);
        free(gpio);
        continue;
      } else if (starts_with(post_data, "on=")) {
        char* gpio = rm_syms(post_data, 3);
        if (gpio_set_active(gpio, 1)) {
          http_plain("200 OK", request.out);
          FCGX_PutS("ok\n", request.out);
        } else {
          http_plain("500 Internal Server Error", request.out);
          FCGX_PutS("error\n", request.out);
        }
        free(post_data);
        free(gpio);
        continue;
      }
      free(post_data);
    }
    http_plain("501 Not Implemented", request.out);
    FCGX_PutS("Invalid request.", request.out);
  }
}
