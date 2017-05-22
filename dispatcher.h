#ifndef DISPATCHER_H
#define DISPATCHER_H

// String constants
#define COUNTER_FILENAME "counter"

#define USAGE "Usage: %s <character> <filename>\n"
#define STAT_FAIL "Failed to read file \"%s\" status: %s\n"
#define FORK_FAIL "Failed to instantiate process!\n"
#define EXECV_FAIL "Failed to execute \"%s\".\n"

#define D_FILE_AND_COUNTERS "Filesize: %ld, Counters: %d\n"
#define D_READ_PIPE "Read \"%s\" from pipe \"%s\"\n"

#endif // !DISPATCHER_H
