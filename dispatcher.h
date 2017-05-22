#ifndef DISPATCHER_H
#define DISPATCHER_H

// Number constants
#define MAX_COUNTERS 16
#define MIN_COUNTARE_SIZE 4096

// String constants
#define COUNTER_FILENAME "counter"

#define USAGE "Usage: %s <character> <filename>\n"
#define STAT_FAIL "Failed to read file '%s' status: %s\n"
#define SIGACTION_FAIL "Failed to register signal handler: %s"
#define FORK_FAIL "Failed to instantiate process!\n"
#define EXECV_FAIL "Failed to execute \"%s\".\n"

#define D_FILE_AND_COUNTERS "Filesize: %ld, Counters: %d\n"
#define D_SIGNAL_RECIEVE "Signal recieved from process %lu\n"

#endif // !DISPATCHER_H
