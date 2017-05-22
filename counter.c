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
int main(int argc, char *argv[]) {
  int parent_pid = getppid();
  kill(parent_pid, SIGUSR1);
}
