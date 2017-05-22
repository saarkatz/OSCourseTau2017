#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#include "counter.h"
#include "definitions.h"

// Global varialble to indicate that parent has recieved the signal
bool parentRecieved = false;

// USR1 signal handler.
// Signal handler that sets parentRecieved to true after recieving signal from
// parnet
void USR1_signal_handler(int signum, siginfo_t *info, void *ptr) {
  PRINT_D(D_SIGNAL_RECIEVE, (unsigned long)info->si_pid);
  parentRecieved = true;
}

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
  struct sigaction USR1Action;
  long offset;
  long size;

  char *str_end;

  int i;

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

  // Register SIGUSR1 signal handler
  memset(&USR1Action, 0, sizeof(USR1Action));
  USR1Action.sa_sigaction = USR1_signal_handler;
  USR1Action.sa_flags = SA_SIGINFO;

  if (sigaction(SIGUSR1, &USR1Action, NULL) < 0) {
    PRINT_I(SIGACTION_FAIL, strerror(errno));
    exit(1);
  }


  // Send SIGUSR1 to parent until an answer is returned or twice the maximum
  // number of counters
  int parent_pid = getppid();
  if (parent_pid > 1) {
    for (i = 0; i < 2*MAX_COUNTERS && !parentRecieved; i++) {
      kill(parent_pid, SIGUSR1);
      sleep(1);
    }
    if (!parentRecieved) {
      // Print mesage if parent did not return
      PRINT_D(D_NO_ANS, pid, parent_pid);
    }
  }

  // Close and delete the pipe
  // ------------------------
}
