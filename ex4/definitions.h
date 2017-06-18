#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

// Debug flags
#define DEBUG 1
//#define DEBUG_LOG_FILE "log.txt"

// Connection
#define PORT 2233
#define SERVER_IP "127.0.0.1"

// Numerical constants
#define MAX_STRING 1024
#define MAX_READ_BUFFER 1025
#define MAX_WRITE_BUFFER (MAX_READ_BUFFER-1)
#define MAX_BACKLOG 10

#define EXIT_SUCC 0
#define EXIT_FAIL 1

// String constants
#define CLIENT_DATASOURCE "/dev/urandom"

// Print strings
#define USE_MAX_LEN_INSTEAD "Using LONG_MAX (%ld) instead.\n"
#define CLIENT_RESULT "%ld characters where sent of which %ld characters are \
readable.\n"
#define TOTAL_COUNT "A total of %ld characters were recieved.\n"
#define WE_SAW "we saw"
#define AMOUNT_OF_CHAR " %ld '%c's"

#define ERR_SIGHANDLE_FAIL "Error while registering signal handler: %s\n"
#define ERR_INIT_MUTEX "Error initializing mutex: %s\n"
#define ERR_INIT_COND "Error initializing condition: %s\n"
#define ERR_VALUE_ERANGE "%s value is out of range (%s %ld)!\n"
#define ERR_CREATE_SOCKET "Error creating socket: %s\n"
#define ERR_CONNECTION_FAIL "Error connecting: %s\n"
#define ERR_BIND_FAIL "Error binding socket: %s\n"
#define ERR_LISTEN_FAIL "Error listening to socket: %s\n"
#define ERR_ACCEPT_FAIL "Error accepting connection: %s\n"
#define ERR_WRITE_SOCK "Error writing to socket: %s\n"
#define ERR_READ_SOCK "Error reading from socket: %s\n"
#define ERR_OPEN_FILE "Error opening file '%s': %s\n"
#define ERR_READ_FILE "Error reading from file: %s\n"

// Print macros
#define PRINT_I(...) do { printf(__VA_ARGS__); } while(0)

#if DEBUG == 0
#define PRINT_D(...) 
#else
#ifndef DEBUG_LOG_FILE
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
snprintf(__printd_buff__, sizeof(__printd_buff__), __VA_ARGS__);\
printf("%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__); } while (0)
#else
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
char __printd_buff2__[MAX_STRING];\
int __printd_fd__ = open(DEBUG_LOG_FILE, O_WRONLY | O_CREAT | O_APPEND);\
snprintf(__printd_buff__, sizeof(__printd_buff__), __VA_ARGS__);\
snprintf(__printd_buff2__, sizeof(__printd_buff2__), "%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__);\
write(__printd_fd__, __printd_buff2__, strlen(__printd_buff2__));\
close(__printd_fd__); } while(0)
#endif
#endif

// Helper macros
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#endif // DEFINITIONS_H
