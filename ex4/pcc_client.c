#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "definitions.h"
#include "pcc_client.h"

// This is the main entry point for the client.
// Usage: pcc_client LEN [-f SOURCE_FILENAME]
//   The client recieves as an argument LEN - the number of bytes to send to
//   the server. It then reads LEN bytes from /dev/urandom and sends them to
//   the server and waits to get an answer from the server with the number of 
//   readable characters. Finaly it prints the answer and exits.
int main(int argc, char *argv[]) {
  // Define variables
  char source_filename[MAX_STRING];
  long len;

  char *end_ptr;

  int sockfd = -1;
  int sourcefd = -1;

  int bytes_written = 0;
  int bytes_read = 0;
  int bytes_left = 0;
  int syscall_return;
  char read_buff[MAX_WRITE_BUFFER + 1];

  long server_result;

  struct sockaddr_in serv_addr;
  struct sockaddr_in my_addr;
  struct sockaddr_in peer_addr;
  socklen_t addrsize = sizeof(struct sockaddr_in);

  // Handle arguments
  if (argc < 2) {
    PRINT_I(USAGE, argv[0]);
    exit(EXIT_FAIL);
  }
  if (argc > 2) {
    if (0 != strcmp(argv[2], ARG_FILE) || 4 != argc) {
      PRINT_I(USAGE, argv[0]);
      exit(EXIT_FAIL);
    }
    else {
      strncpy(source_filename, argv[3],
        MIN(MAX_STRING, strlen(argv[3])));
    }
  }
  else {
    strncpy(source_filename, CLIENT_DATASOURCE,
      MIN(MAX_STRING, strlen(CLIENT_DATASOURCE)));
  }

  len = strtol(argv[1], &end_ptr, 0);
  if (*end_ptr != '\0' || len < 1) {
    PRINT_I(USAGE, argv[0]);
    exit(EXIT_FAIL);
  }
  if (ERANGE == errno) {
    PRINT_I(ERR_VALUE_ERANGE, "LEN", "MAX", len);
    PRINT_I(USE_MAX_LEN_INSTEAD, len);
  }

  // Open target file for reading
  if ((sourcefd = open(source_filename, O_RDONLY)) < 0) {
    PRINT_I(ERR_OPEN_FILE, source_filename, strerror(errno));
    exit(EXIT_FAIL);
  }

  // Setup socket connection
  memset(read_buff, '\0', sizeof(read_buff));

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    PRINT_I(ERR_CREATE_SOCKET, strerror(errno));
    close(sourcefd);
    exit(EXIT_FAIL);
  }

  memset(&serv_addr, '\0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  PRINT_D(D_CLIENT_CONNETING);

  if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    PRINT_I(ERR_CONNECTION_FAIL, strerror(errno));
    close(sourcefd);
    close(sockfd);
    exit(EXIT_FAIL);
  }

  PRINT_D("Sending message length to server.\n");

  len = htonl(len);
  syscall_return = write(sockfd, &len, sizeof(len));
  if (syscall_return < 0) {
    PRINT_I(ERR_WRITE_SOCK, strerror(errno));
    close(sourcefd);
    close(sockfd);
    exit(EXIT_FAIL);
  }
  len = ntohl(len);

  PRINT_D("Sending data to server.\n");

  // Send data to server
  while (bytes_written < len) {
    // Read a buffer-full of data from file up to the total
    syscall_return = read(sourcefd, read_buff,
      MIN(MAX_WRITE_BUFFER, len - bytes_written));
    if (syscall_return < 0) {
      PRINT_I(ERR_READ_FILE, strerror(errno));
      close(sourcefd);
      close(sockfd);
      exit(EXIT_FAIL);
    }

    read_buff[syscall_return] = '\0';
    bytes_read = syscall_return;
    bytes_left = 0;

    // Write the buffer to server
    while (0 < bytes_read) {
      syscall_return = write(sockfd, read_buff + bytes_left,
        MIN(MAX_WRITE_BUFFER, bytes_read));
      if (syscall_return < 0) {
        PRINT_I(ERR_WRITE_SOCK, strerror(errno));
        close(sourcefd);
        close(sockfd);
        exit(EXIT_FAIL);
      }

      bytes_left += syscall_return;
      bytes_read -= syscall_return;
    }
    bytes_written += bytes_left;
  }

  PRINT_D("Finished sending data\n");

  // Read result from the server
  syscall_return = read(sockfd, &server_result, sizeof(server_result));
  if (syscall_return < 0) {
    PRINT_I(ERR_READ_SOCK, strerror(errno));
    close(sourcefd);
    close(sockfd);
    exit(EXIT_FAIL);
  }
  else if (syscall_return == 0) {
    PRINT_D("Server closed connection but result was not recieved\n");
    close(sourcefd);
    close(sockfd);
    exit(EXIT_FAIL);
  }
  server_result = ntohl(server_result);
  PRINT_I(CLIENT_RESULT, len, server_result);
  PRINT_D("Closing connection.\n");
  close(sourcefd);
  close(sockfd);
}