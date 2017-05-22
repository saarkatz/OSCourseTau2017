#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "dispatcher.h"
#include "definitions.h"

// USR1 signal handler.
// This signal sould be sent by a counter process to notifie the dispacher that
// it has finished to counts its chanck of the file.
void USR1_signal_handler(int signum, siginfo_t *info, void *ptr) {
  PRINT_I("Signal sent from process %lu\n", (unsigned long)info->si_pid);
}

// Main entry point for the dispatcher
// Thee dispacher launches several counter processes after creating a pipe
// trough which they will report their count. The dispacher will sum the
// results of the counters and print the result.
// Command line arguments - 
//  argv[1] - the character to count
//  argv[2] - the text file to process
int main(int argc, char *argv[]) {
  // Define variables
  struct stat fileStat;
  int numCounters;

  // Validate usage
  if (argc != 3 || strlen(argv[1]) != 1) {
    PRINT_I(USAGE, argv[0]);
    exit(0);
  }
  
  fileStat.st_size = atoi(argv[2]);
//  // Determain the size of the file - use stat
//  if (stat(argv[2], &fileStat) < 0) {
//    PRINT_I(STAT_FAIL, argv[2], strerror(errno));
//    exit(1);
//  }

  // Choose the number of counters to use.
  numCounters = fileStat.st_size / MIN_COUNTARE_SIZE;
  if (numCounters < 1) {
    numCounters = 1;
  }
  else if (numCounters > MAX_COUNTERS) {
    numCounters = MAX_COUNTERS;
  }

  PRINT_D("Filesize: %d, Counters: %d\n", fileStat.st_size, numCounters);

}