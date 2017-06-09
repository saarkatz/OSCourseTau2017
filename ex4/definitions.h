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
#define MAX_WRITE_BUFFER 1025
#define MAX_READ_BUFFER (MAX_WRITE_BUFFER-1)

#define EXIT_SUCC 0
#define EXIT_FAIL 1

// String constants
#define CLIENT_DATASOURCE "/dev/urandom"


// Print strings
#define USE_MAX_LEN_INSTEAD "Using LONG_MAX (%ld) instead.\n"

#define ERR_VALUE_ERANGE "%s value is out of range (%s %ld)!\n"

// Print macros
#define PRINT_I(...) do { printf(__VA_ARGS__); } while(0)

#if DEBUG == 0
#define PRINT_D(...) 
#else
#ifndef DEBUG_LOG_FILE
#define PRINT_D(...) do {\
char __printd_buff__[MAX_STRING];\
sprintf(__printd_buff__, __VA_ARGS__);\
printf("%-13.13s - %-4d: %s", __FILE__, __LINE__, __printd_buff__); } while (0)
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

// Helper macros
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#endif // !DEFINITIONS_H
