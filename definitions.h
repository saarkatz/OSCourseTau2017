#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Number constants
#define MAX_COUNTERS 16
#define MIN_COUNTARE_SIZE 4096
#define MAX_STRING 1024
#define MAX_ARG_LENGTH 256

// String constants
#define PIPENAME_FORMAT "/tmp/counter_%d"

#define SIGACTION_FAIL "Failed to register signal handler: %s\n"

#define D_SIGNAL_RECIEVE "Signal recieved from process %lu\n"

// Print macros
#define PRINT_I(...) do { printf(__VA_ARGS__); } while(0)
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
sprintf(__printd_buff__, __VA_ARGS__);\
printf("%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__); } while(0)

#endif // !DEFINITIONS_H
