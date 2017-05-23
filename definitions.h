#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

// Debug flags
#define DEBUG 0
//#define DEBUG_LOG_FILE "log.txt"

// Number constants
#define MAX_COUNTERS 16
#define MIN_COUNTARE_PAGE_NUM 2
#define MAX_STRING 1024
#define SIGNAL_TRIES 2 * MAX_COUNTERS
#define USLEEP_SIGNALS 50000

#define MAX_ARG_LENGTH 256
#define PIPE_MODE 0666

// String constants
#define PIPENAME_FORMAT "/tmp/counter_%d"
#define RESULT_FORMAT "Counted %lu appearances of char %c in file \"%s\"\n"
#define RESULT_NOT_EXACT "This result is not exact.\n"\
                          "The following chuncks are missing from the count\n"
#define MISSING_CHUNK "Chunck from %ld to %ld\n"

#define SIGACTION_FAIL "Failed to register signal handler: %s\n"
#define CREATE_PIPE_FAIL "Failed to create pipe \"%s\": %s\n"
#define OPEN_PIPE_FAIL "Failed to open pipe \"%s\": %s\n"
#define READ_PIPE_FAIL "Failed to read from pipe \"%s\": %s\n"
#define WRITE_PIPE_FAIL "Failed to write to pipe \"%s\": %s\n"
#define OPEN_FILE_FAIL "Failed to open file \"%s\": %s\n"
#define CREATE_MAP_FAIL "Failed to map file to memory: %s\n"
#define COUNTER_FAIL "An error occured while waiting for a counter process "\
                      "%d\n"

#define D_SIGNAL_RECIEVE "Signal recieved from process %lu\n"
#define D_RETRY_PROCESS "Retrying process %d\n"

// Print macros
#define PRINT_I(...) do { printf(__VA_ARGS__); } while(0)

#if DEBUG == 0
#define PRINT_D(...) 
#else
#ifndef DEBUG_LOG_FILE
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
sprintf(__printd_buff__, __VA_ARGS__);\
printf("%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__); }while (0)
#else
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
char __printd_buff2__[MAX_STRING];\
int __printd_fd__ = open(DEBUG_LOG_FILE, O_WRONLY | O_CREAT | O_APPEND);\
sprintf(__printd_buff__, __VA_ARGS__);\
sprintf(__printd_buff2__, "%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__);\
write(__printd_fd__, __printd_buff2__, strlen(__printd_buff2__));\
close(__printd_fd__); } while(0)
#endif
#endif

// Helper macro
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CEIL_DIV(x, y) (((x) % (y)) ? (x) / (y) + 1 : (x) / (y))

#endif // !DEFINITIONS_H
