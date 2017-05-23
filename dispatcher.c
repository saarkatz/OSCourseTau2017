#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

#include "dispatcher.h"
#include "definitions.h"

// Data structure to keep track of running processes
typedef struct proc {
  int pid;
  long offset;
  long size;
  bool received;
  bool stopped;
  bool clean_finish;
} PROC;

// Global counter
unsigned long count = 0;

// Array of procEntry
PROC counters[MAX_COUNTERS];

// Keeps the head of couters array
int currentProc = 0;

// USR1 signal handler.
// This signal sould be sent by a counter process to notifie the dispacher that
// it has finished to counts its chanck of the file.
void USR1_signal_handler(int signum, siginfo_t *info, void *ptr) {
  PRINT_D(D_SIGNAL_RECIEVE, (unsigned long)info->si_pid);
  char pipename[MAX_STRING];
  char buffer[MAX_STRING];
  int pd;

  int i;

  long amount;
  char *str_end;

  // Check that a signal from this counter was not received yet
  for (i = 0; i < currentProc; i++) {
    if (info->si_pid == counters[i].pid) {
      if (counters[i].received) {
        return;
      }
      else {
        counters[i].received = true;
        break;
      }

      if (i == currentProc - 1) {
        // If we get here then we received the signal before the dispatcher
        // Managed to update the array and currentProc. In this case we return
        PRINT_D("Process %d was not registered yet\n", info->si_pid);
        return;
      }
    }
  }

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
  bool finished = false;

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

    // Register the new process
    counters[currentProc].pid = p;
    counters[currentProc].offset = curOffset;
    counters[currentProc].size = MIN(curChanckSize * systemPageSize, fileSize);
    counters[currentProc].received = false;
    counters[currentProc].stopped = false;
    counters[currentProc].clean_finish = false;
    currentProc++;

    curOffset += curChanckSize * systemPageSize;
    fileSizeInPages -= curChanckSize;
    fileSize -= curChanckSize * systemPageSize;
  }

  // Wait for all the counters to finish
  while (!finished) {
    p = wait(&wstatus);
    if (p == -1) {
      if (ECHILD == errno) {
        // No more unwaited children, Check if we finished.
        for (i = 0; i < currentProc; i++) {
          if (false == counters[i].stopped) break;
          if (currentProc - 1 == i) finished = true;
        }
        if (finished) {
          PRINT_D("All processes finished but it was not cought by the "
            "loop\n");
          break;
        }
      }
      else if (EINTR == errno) {
        // Interrupted by signal
        continue;
      }
      // An error occured
      PRINT_I("Failed to wait for child: %s\n", strerror(errno));
      break;
    }

    // At this point we know we got the return of a child process.
    for (i = 0; i < currentProc; i++) {
      if (p == counters[i].pid) break;
    }
    if (i == currentProc) {
      // Counter not registered?!
      PRINT_D("Counter %d is not registered in table\n", p);
      break;
    }

    if (!WIFEXITED(wstatus) || 0 != WEXITSTATUS(wstatus)) {
      PRINT_I(COUNTER_FAIL, p);
      counters[i].clean_finish = false;
    }
    else {
      counters[i].clean_finish = true;
    }
    counters[i].stopped = true;

    if (false == counters[i].received) {
      PRINT_D("Signal was not received from counter %d\n", p);
      counters[i].clean_finish = false;
    }

    // Check if all child processes have finished
    for (i = 0; i < currentProc; i++) {
      if (false == counters[i].stopped) break;
      if (currentProc - 1 == i) finished = true;
    }
  }

  PRINT_I(RESULT_FORMAT, count, argv[1][0], argv[2]);
  // Check to see if any counter returned error value. If so print a report
  // accordingly
  for (i = 0; i < currentProc; i++) {
    if (false == counters[i].clean_finish) {
      if (finished) {
        PRINT_I(RESULT_NOT_EXACT);
        finished = false;
      }
      PRINT_I(MISSING_CHUNK, counters[i].offset,
        counters[i].offset + counters[i].size - 1);
    }
    
  }
  exit(0);
}