#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "dispatcher.h"
#include "definitions.h"

// Global counter
unsigned long count = 0;

// USR1 signal handler.
// This signal sould be sent by a counter process to notifie the dispacher that
// it has finished to counts its chanck of the file.
void USR1_signal_handler(int signum, siginfo_t *info, void *ptr) {
  PRINT_D(D_SIGNAL_RECIEVE, (unsigned long)info->si_pid);
  char pipename[MAX_STRING];
  char buffer[MAX_STRING];
  int pd;

  long amount;
  char *str_end;

  // Send signal back to child to notify it that signal was recieved
  kill(info->si_pid, SIGUSR1);

  // Create pipename
  sprintf(pipename, PIPENAME_FORMAT, info->si_pid);

  // Open pipe
  pd = open(pipename, O_RDONLY);
  if (pd < 0) {
    PRINT_I(OPEN_PIPE_FAIL, pipename, strerror(errno));
    return;
  }

  // Read from pipe
  if (read(pd, buffer, MAX_STRING) < 0) {
    PRINT_I(READ_PIPE_FAIL, pipename, strerror(errno));
    close(pd);
    return;
  }

  // Get the number from the buffer
  PRINT_D(D_READ_PIPE, buffer, pipename);
  amount = strtol(buffer, &str_end, 0);
  if (str_end == buffer) {
    PRINT_I(READ_PIPE_FAIL, pipename, strerror(errno));
    close(pd);
    return;
  }

  // Add the number to the counter
  count += amount;

  // Close the pipe
  close(pd);
}

// Main entry point for the dispatcher
// The dispacher launches several counter processes after creating a pipe
// trough which they will report their count. The dispacher will sum the
// results of the counters and print the result.
// Command line arguments - 
//  argv[1] - the character to count
//  argv[2] - the text file to process
int main(int argc, char *argv[]) {
  // Define variables
  struct sigaction USR1Action;
  struct stat fileStat;
  int fileSizeInPages;
  long fileSize;
  int numCounters;

  char arg3[MAX_ARG_LENGTH];
  char arg4[MAX_ARG_LENGTH];
  char *in_args[] = { COUNTER_FILENAME, argv[1], argv[2], arg3, arg4, NULL };
  int p;

  long curOffset;
  long curChanckSize;

  int i;
  int wstatus;

  long systemPageSize = sysconf(_SC_PAGE_SIZE);

  // Validate arguments
  if (argc != 3 || strlen(argv[1]) != 1) {
    PRINT_I(USAGE, argv[0]);
    exit(0);
  }
  
  // Determain the size of the file - use stat
  if (stat(argv[2], &fileStat) < 0) {
    PRINT_I(STAT_FAIL, argv[2], strerror(errno));
    exit(1);
  }

  // Get file size (in pages and rounded up)
  fileSizeInPages = CEIL_DIV(fileStat.st_size, systemPageSize);
  
  // Choose the number of counters to use.
  numCounters = fileSizeInPages / MIN_COUNTARE_PAGE_NUM;
  if (numCounters < 1) {
    numCounters = 1;
  }
  else if (numCounters > MAX_COUNTERS) {
    numCounters = MAX_COUNTERS;
  }

  PRINT_D(D_FILE_AND_COUNTERS, (long)fileStat.st_size, fileSizeInPages, numCounters);

  // Register SIGUSR1 signal handler
  memset(&USR1Action, 0, sizeof(USR1Action));
  USR1Action.sa_sigaction = USR1_signal_handler;
  USR1Action.sa_flags = SA_SIGINFO;

  if (sigaction(SIGUSR1, &USR1Action, NULL) < 0) {
    PRINT_I(SIGACTION_FAIL, strerror(errno));
    exit(1);
  }

  curOffset = 0;
  fileSize = fileStat.st_size;
  for (i = 0; i < numCounters; i++) {
    PRINT_D("Instantiating counter number %d\n", i);

    curChanckSize = CEIL_DIV(fileSizeInPages, numCounters - i);

    // Generate arguments for the next counter
    sprintf(in_args[3], "%ld", curOffset);
    sprintf(in_args[4], "%ld", MIN(curChanckSize * systemPageSize, fileSize));

    // Execute an instace of counter
    if (-1 == (p = fork())) {
      PRINT_I(FORK_FAIL);
      exit(1);
    }
    else if (0 == p) {
      // We are the child
      execv(COUNTER_FILENAME, in_args);
      PRINT_I(EXECV_FAIL, COUNTER_FILENAME);
      exit(1);
    }

    curOffset += curChanckSize * systemPageSize;
    fileSizeInPages -= curChanckSize;
    fileSize -= curChanckSize * systemPageSize;
  }

  for (i = 0; i < numCounters; i++){
    p = wait(&wstatus);
    if (!WIFEXITED(wstatus) || 0 != WEXITSTATUS(wstatus)) {
      PRINT_I(COUNTER_FAIL, p);
    }
  }

  PRINT_I(RESULT_FORMAT, count);
  exit(0);
}