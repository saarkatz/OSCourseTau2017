#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "message_slot_c.h"

#define DEBUG 1

#define USAGE "Usage: %s [-d <device name>] <channel number>\n"
#define EXIT_FAIL 1
#define EXIT_SUCC 0
#define MAX_BUF 1024

#define ARG_DEVICE "-d"

// Print macros
#define PRINT_I(...) do { printf(__VA_ARGS__); } while(0)

#if DEBUG == 0
#define PRINT_D(...) 
#else
#ifndef DEBUG_LOG_FILE
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
snprintf(__printd_buff__, sizeof(__printd_buff__), __VA_ARGS__);\
printf("%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__); } while (0)
#endif
#endif


// This code reads a message from a message_slot managed device.
// Usage: <name> [-d <device name>] <channel number>
int main(int argc, char *argv[]) {
  long channel;
  char *end_ptr;
  char device_name[MAX_BUF] = DEVICE_FOLDER "/" DEVICE_RANGE_NAME;
  int device;
  int result;
  char message[BUF_LEN + 1];

  // Handle arguments
  if (argc < 2) {
    PRINT_I(USAGE, argv[0]);
    exit(EXIT_FAIL);
  }
  if (argc > 2) {
    if (4 != argc || 0 != strcmp(argv[1], ARG_DEVICE)) {
      PRINT_I(USAGE, argv[0]);
      exit(EXIT_FAIL);
    }
    else {
      if (strlen(argv[2]) > MAX_BUF) {
        PRINT_I("Device name too long (max length: %d)\n", MAX_BUF);
        exit(EXIT_FAIL);
      }
      strncpy(device_name, argv[2], strlen(argv[2]));

      channel = strtol(argv[3], &end_ptr, 0);
      if (*end_ptr != '\0' || channel < 0) {
        PRINT_I(USAGE, argv[0]);
        exit(EXIT_FAIL);
      }
      if (ERANGE == errno) {
        PRINT_I("Channel number out of range (%ld)\n", channel);
        exit(EXIT_FAIL);
      }
    }
  }
  else {
    channel = strtol(argv[1], &end_ptr, 0);
    if (*end_ptr != '\0' || channel < 0) {
      PRINT_I(USAGE, argv[0]);
      exit(EXIT_FAIL);
    }
    if (ERANGE == errno) {
      PRINT_I("Channel number out of range (%ld)\n", channel);
      exit(EXIT_FAIL);
    }
  }


  // Read message from device
  // Open device
  device = open(device_name, O_RDONLY);
  if (0 > device) {
    PRINT_I("Couldn't open device '%s': %s\n", device_name, strerror(errno));
    exit(EXIT_FAIL);
  }
  // Ioctl: Set channel
  result = ioctl(device, IOCTL_SET_CHAN, channel);
  if (0 > result) {
    PRINT_I("Error using ioctl on device '%s': %s\n", device_name,
      strerror(errno));
    close(device);
    exit(EXIT_FAIL);
  }
  // Read message to buffer
  result = read(device, message, BUF_LEN);
  if (0 > result) {
    PRINT_I("Error reading from device '%s': %s\n", device_name,
      strerror(errno));
    close(device);
    exit(EXIT_FAIL);
  }

  message[result] = '\0'; // Terminate the string

  PRINT_I("Message: %s\n", message);
  PRINT_I("Read was succesful! %d bytes were read.\n", result);
}