#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "definitions.h"
#include "pcc_client.h"

// This is the main entry point for the client.
// Usage: pcc_client LEN [-f SOURCE_FILENAME]
//   The client recieves as an argument LEN - the number of bytes to send to
//   the server. It then reads LEN bytes from /dev/urandom and sends them to
//   the server and waits to get an answer from the server with the number of 
//   readable characters. Finaly it prints the answer and exits.
int main(int argc, char *argv[]) {
  // Define variables
  char source_filename[MAX_STRING];
  long len;

  char *end_ptr;

  // Handle arguments
  if (argc < 2) {
    PRINT_I("1 " USAGE, argv[0]);
    exit(EXIT_FAIL);
  }
  if (argc > 2) {
    if (0 != strcmp(argv[2], ARG_FILE) || 4 != argc) {
      PRINT_I("2 " USAGE, argv[0]);
      exit(EXIT_FAIL);
    }
    else {
      strncpy(source_filename, argv[3],
        MIN(MAX_STRING, strlen(argv[3])));
    }
  }
  else {
    strncpy(source_filename, CLIENT_DATASOURCE,
      MIN(MAX_STRING, strlen(CLIENT_DATASOURCE)));
  }

  len = strtol(argv[1], &end_ptr, 0);
  if (*end_ptr != '\0' || len < 1) {
    PRINT_I("3 " USAGE, argv[0]);
    exit(EXIT_FAIL);
  }
  if (ERANGE == errno) {
    PRINT_I(ERR_VALUE_ERANGE, "LEN", "MAX", len);
    PRINT_I(USE_MAX_LEN_INSTEAD, len);
  }

  PRINT_D("File: %s\n", source_filename);
  PRINT_D("len: %ld\n", len);
}