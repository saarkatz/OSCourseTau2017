#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "counter.h"
#include "definitions.h"

// Main entry point for the counter
// The counter counts the number of times a character, stored in argv[1]
// appears in file argv[2], starting at offset argv[3] and until 
// argv[3] + argv[4]. The counter writes the result to a pipe named 
// /tmp/counter_<PID> where <PID> is the process id of the counter, after which
// it's sends the signal SIGUSR1 to the parent process.
//  argv[1] – the character to count
//  argv[2] – the text file to process
//  argv[3] – the offset in the text file to start counting from
//  argv[4] – the length of the relevant chunk to process
//
// Note: Don't attempt to run from terminal without commenting kill() because
//       SIGUSR1 apparently kills the teminal. 
int main(int argc, char *argv[]) {
  // Define variables
  long offset;
  long size;

  char *str_end;

  int pid = getpid();

  // Validate arguments
  if (argc != 5 || strlen(argv[1]) != 1) {
    PRINT_D("%d: " USAGE, pid, argv[0]);
    exit(0);
  }

  // Make sure that argv[3] and argv[4] are numbers.
  offset = strtol(argv[3], &str_end, 0);
  if (argv[3] + strlen(argv[3]) != str_end && 1 != strlen(argv[3])) {
    PRINT_D("%d: Not a number: %s\n", pid, argv[3]);
    PRINT_I(USAGE, argv[0]);
    exit(0);
  }
  size = strtol(argv[4], &str_end, 0);
  if (argv[4] + strlen(argv[4]) != str_end) {
    PRINT_D("%d: Not a number: %s\n", pid, argv[3]);
    PRINT_I(USAGE, argv[0]);
    exit(0);
  }

  PRINT_D("%d: Offset %ld, size %ld\n", pid, offset, size);

  int parent_pid = getppid();
  if (parent_pid > 1) {
    PRINT_D("%d: Parent pid: %ld\n", pid, (long)parent_pid);
    kill(parent_pid, SIGUSR1);
  }
}
