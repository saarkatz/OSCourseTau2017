#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#include "definitions.h"
#include "pcc_server.h"

typedef struct count_statistics {
  long count;
} Statistics;

int listenfd = -1;

void INT_signal_handler(int signum) {
  PRINT_D("Closing server\n");
  close(listenfd);
  exit(EXIT_SUCC);
}

void *client_routine(void *vconnfd) {
  int connfd = (int)vconnfd;
  int syscall_return;

  char buff[MAX_WRITE_BUFFER + 1];
  long message_length = 0;
  long bytes_read = 0;
  long buff_head = 0;

  Statistics stats;

  int exit_fail = EXIT_FAIL;
  int i = 0;

  // Read message length from socket
  syscall_return = read(connfd, &message_length, sizeof(message_length));
  if (syscall_return < 0) {
    PRINT_D("C:%d" ERR_READ_SOCK, connfd, strerror(errno));
    PRINT_I(ERR_READ_SOCK, strerror(errno));
    close(connfd);
    pthread_exit(&exit_fail);
  }
  else if (syscall_return < sizeof(message_length)) {
    PRINT_D("C:%d: Not all the number was read! got %d bytes but expected %d "
      "bytes.\n", connfd, syscall_return, sizeof(message_length));
    close(connfd);
    pthread_exit(&exit_fail);
  }

  message_length = ntohl(message_length);

  PRINT_D("C:%d: Message length: %ld\n", connfd, message_length);

  // Recieve message
  while (bytes_read < message_length) {
    buff_head = 0;
    // Read a buffer full of data from the socket, up to message length
    while (buff_head < MIN(MAX_WRITE_BUFFER, message_length - bytes_read)) {
      syscall_return = read(connfd, buff + buff_head,
        MIN(MAX_WRITE_BUFFER - buff_head, message_length - bytes_read));
      if (syscall_return < 0) {
        PRINT_D("C:%d" ERR_READ_SOCK, connfd, strerror(errno));
        PRINT_I(ERR_READ_SOCK, strerror(errno));
        close(connfd);
        pthread_exit(&exit_fail);
      }
      else if (0 == syscall_return) {
        PRINT_D("C:%d Peer has disconnected\n" , connfd);
        close(connfd);
        pthread_exit(&exit_fail);
      }
      buff_head += syscall_return;
    }
    buff[buff_head] = '\0';
    bytes_read += buff_head;

    // Count readable characters
    for (i = 0; i < buff_head; i++) {
      if (buff[i] >= 32 && buff[i] <= 126) {
        stats.count++;
      }
    }
  }

  PRINT_D("%ld bytes were readble\n", stats.count);

  // Send result back to client
  stats.count = htonl(stats.count);
  syscall_return = write(connfd, &stats.count, sizeof(stats.count));
  if (syscall_return < 0) {
    PRINT_D("C:%d " ERR_WRITE_SOCK, connfd, strerror(errno));
    PRINT_I(ERR_WRITE_SOCK, strerror(errno));
    close(connfd);
    pthread_exit(&exit_fail);
  }
  stats.count = ntohl(stats.count);

  PRINT_D("C:%d Closing connection to peer\n", connfd);

  close(connfd);
}

// Start a server that listen on port PORT and accepts clients to recieve data
// from
int main(int argc, char *argv[]) {
  struct sigaction INTHandle;
  int connfd = -1;

  struct sockaddr_in serv_addr;
  struct sockaddr_in peer_addr;

  char data_buff[MAX_WRITE_BUFFER + 1];

  int syscall_return;

  // Register SIGINT signal handler
  memset(&INTHandle, 0, sizeof(INTHandle));
  INTHandle.sa_handler = INT_signal_handler;

  if (sigaction(SIGINT, &INTHandle, NULL) < 0) {
    PRINT_I(ERR_SIGHANDLE_FAIL, strerror(errno));
    exit(EXIT_FAIL);
  }

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    PRINT_I(ERR_CREATE_SOCKET, strerror(errno));
    exit(EXIT_FAIL);
  }
  memset(&serv_addr, '\0', sizeof(serv_addr));
  memset(data_buff, '\0', sizeof(data_buff));

  serv_addr.sin_family = AF_INET;
  // INADDR_ANY = any local machine address
  serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  serv_addr.sin_port = htons(PORT);

  if (0 != bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) {
    PRINT_I(ERR_BIND_FAIL, strerror(errno));
    close(listenfd);
    exit(EXIT_FAIL);
  }

  if (0 != listen(listenfd, MAX_BACKLOG)) {
    PRINT_I(ERR_LISTEN_FAIL, strerror(errno));
    close(listenfd);
    exit(EXIT_FAIL);
  }

  PRINT_D("Finished setting up connection.\n");
  PRINT_D("Starting to listen...\n");

  // Work loop
  while (1)
  {
    pthread_t thread;

    // Accept a connection.
    connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
      PRINT_I(ERR_ACCEPT_FAIL, strerror(errno));
      exit(EXIT_FAIL); // Probably should exit differently
    }

    PRINT_D("Recieved connection. (Connfd %d)\n", connfd);

    syscall_return = pthread_create(&thread, NULL, client_routine,
      (void*)connfd);
  }
}