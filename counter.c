#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
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
long counter;

// USR1 signal handler.
// Signal handler that sets parentRecieved to true after recieving signal from
// parnet
void USR1_signal_handler(int signum, siginfo_t *info, void *ptr) {
  PRINT_D(D_SIGNAL_RECIEVE, (unsigned long)info->si_pid);
  char pipepath[MAX_STRING];
  char buffer[MAX_STRING];
  int pd;
  int pid = getpid();

  parentRecieved = true;
  
  // Create pipename
  sprintf(pipepath, PIPENAME_FORMAT, pid);

  // Create a string for the number
  sprintf(buffer, "%ld", counter);

  pd = open(pipepath, O_WRONLY);
  if (pd < 0) {
    PRINT_D(OPEN_PIPE_FAIL, pipepath, strerror(errno));
  }

  // Write to the pipe
  PRINT_D("%d: " D_WRITING_PIPE, pid, buffer, pipepath);
  if (write(pd, buffer, strlen(buffer)) < 0) {
    PRINT_I(WRITE_PIPE_FAIL, pipepath, strerror(errno));
  }

  // Close pipe
  close(pd);
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
  int fd;
  int pd;
  char *map;
  char pipepath[MAX_STRING];
  long offset;
  long size;

  char *str_end;

  int i;

  int pid = getpid();

  counter = 0;

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

//  // Open the file
//  fd = open(argv[2], O_RDONLY);
//  if (fd < 0) {
//    PRINT_I(OPEN_FILE_FAIL, strerror(errno));
//    exit(1);
//  }

//  // Map the file into memory
//  map = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, offset);
//  if (map == MAP_FAILED) {
//    PRINT_I(CREATE_MAP_FAIL, strerror(errno));
//    exit(1);
//  }

//  // Count the num of appearences of the character in the file
//  for (int i = 0; i < size; i++) {
//    if (map[i] == argv[1][0]) {
//      counter += 1;
//    }
//  }
  counter = offset;

//  close(fd);
//  munmap(map, size);

  // Create pipename
  sprintf(pipepath, PIPENAME_FORMAT, pid);

  // Create the pipe
  pd = mkfifo(pipepath, PIPE_MODE);
  if (pd < 0) {
    PRINT_I(CREATE_PIPE_FAIL, pipepath, strerror(errno));
    exit(1);
  }

  // Send SIGUSR1 to parent until an answer is returned or twice the maximum
  // number of counters tries were tried
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

  // Delete the pipe
  unlink(pipepath);
}
