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
  long total;
  long count;
  long chars[256];
} Statistics;

// Global variables
int listenfd = -1;
Statistics global_stats;
int thread_count = 0;

// Mutex for global variable
pthread_mutex_t lock_stats;
pthread_mutex_t lock_count;
pthread_cond_t cond_all_finish;

void INT_signal_handler(int signum) {
  int i;

  PRINT_D("Interrupt signal recieved, waiting for threads to finish\n");
  
  pthread_mutex_lock(&lock_count);
  while (thread_count > 0) {
    PRINT_D("There are thread that need to be waited\n");
    pthread_cond_wait(&cond_all_finish, &lock_count);
  }
  pthread_mutex_unlock(&lock_count);


  // Print statistics
  pthread_mutex_lock(&lock_stats);
  PRINT_I(TOTAL_COUNT, global_stats.total);
  if (global_stats.total > 0) {
    PRINT_I(WE_SAW);
    i = 128 + ' ';
    while (global_stats.chars[i] <= 0 && i < 128 + '}') i++;
    if (i < 128 + '}') {
      PRINT_I(AMOUNT_OF_CHAR, global_stats.chars[i], (char)(i - 128));
    }
    i++;
    for (; i < 128 + '}'; i++) {
      if (global_stats.chars[i] > 0) {
        PRINT_I(","); 
        PRINT_I(AMOUNT_OF_CHAR, global_stats.chars[i], (char)(i - 128));
      }
    }
    PRINT_I("\n");
  }
  pthread_mutex_unlock(&lock_stats);

  PRINT_D("Closing server\n");
  pthread_mutex_destroy(&lock_stats);
  pthread_mutex_destroy(&lock_count);
  pthread_cond_destroy(&cond_all_finish);
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
  int exit_succ = EXIT_SUCC;
  int i = 0;
  int char_num;

  memset(&stats, 0, sizeof(stats));

  // Increment thread counter
  pthread_mutex_lock(&lock_count);
  thread_count++;
  PRINT_D("C:%d thread_count: %d\n", connfd, thread_count);
  pthread_mutex_unlock(&lock_count);
  
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
        PRINT_D("C:%d: " ERR_READ_SOCK, connfd, strerror(errno));
        PRINT_I(ERR_READ_SOCK, strerror(errno));
        close(connfd);
        pthread_exit(&exit_fail);
      }
      else if (0 == syscall_return) {
        PRINT_D("C:%d Client has disconnected\n" , connfd);
        close(connfd);
        pthread_exit(&exit_fail);
      }
      buff_head += syscall_return;
    }
    buff[buff_head] = '\0';
    bytes_read += buff_head;

    // Count readable characters
    stats.total += buff_head;
    for (i = 0; i < buff_head; i++) {
      char_num = 128 + buff[i];
      stats.chars[char_num]++;
      if (buff[i] >= 32 && buff[i] <= 126) {
        stats.count++;
      }
    }
  }

  PRINT_D("C:%d %ld bytes were readble\n", connfd, stats.count);

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

  PRINT_D("C:%d Closing connection to client\n", connfd);

  close(connfd);
  
  // Add results to global data structure
  pthread_mutex_lock(&lock_stats);
  global_stats.total += stats.total;
  for (i = 0; i < 256; i++) {
    global_stats.chars[i] += stats.chars[i];
  }
  pthread_mutex_unlock(&lock_stats);

  // Decrement thread counter
  pthread_mutex_lock(&lock_count);
  thread_count--;
  if (0 == thread_count) {
    pthread_cond_signal(&cond_all_finish);
  }
  pthread_mutex_unlock(&lock_count);

  pthread_exit(&exit_succ);
}

// Start a server that listen on port PORT and accepts clients to recieve data
// from
int main(int argc, char *argv[]) {
  struct sigaction INTHandle;
  pthread_t thread;
  int connfd = -1;

  struct sockaddr_in serv_addr;
  struct sockaddr_in peer_addr;

  char data_buff[MAX_WRITE_BUFFER + 1];

  int syscall_return;

  // Zero out statistics data structure
  memset(&global_stats, 0, sizeof(global_stats));

  // Register SIGINT signal handler
  memset(&INTHandle, 0, sizeof(INTHandle));
  INTHandle.sa_handler = INT_signal_handler;

  if (sigaction(SIGINT, &INTHandle, NULL) < 0) {
    PRINT_I(ERR_SIGHANDLE_FAIL, strerror(errno));
    exit(EXIT_FAIL);
  }

  // Initialze mutex
  if (pthread_mutex_init(&lock_stats, NULL) < 0) {
    PRINT_I(ERR_INIT_MUTEX, strerror(errno));
    exit(EXIT_FAIL);
  }
  if (pthread_mutex_init(&lock_count, NULL) < 0) {
    PRINT_I(ERR_INIT_MUTEX, strerror(errno));
    pthread_mutex_destroy(&lock_stats);
    exit(EXIT_FAIL);
  }
  if (pthread_cond_init(&cond_all_finish, NULL) < 0) {
    PRINT_I(ERR_INIT_COND, strerror(errno));
    pthread_mutex_destroy(&lock_stats);
    pthread_mutex_destroy(&lock_count);
    exit(EXIT_FAIL);
  }

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    PRINT_I(ERR_CREATE_SOCKET, strerror(errno));
    pthread_mutex_destroy(&lock_stats);
    pthread_mutex_destroy(&lock_count);
    pthread_cond_destroy(&cond_all_finish);
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
    pthread_mutex_destroy(&lock_stats);
    pthread_mutex_destroy(&lock_count);
    pthread_cond_destroy(&cond_all_finish);
    close(listenfd);
    exit(EXIT_FAIL);
  }

  if (0 != listen(listenfd, MAX_BACKLOG)) {
    PRINT_I(ERR_LISTEN_FAIL, strerror(errno));
    pthread_mutex_destroy(&lock_stats);
    pthread_mutex_destroy(&lock_count);
    pthread_cond_destroy(&cond_all_finish);
    close(listenfd);
    exit(EXIT_FAIL);
  }

  PRINT_D("Finished setting up connection.\n");
  PRINT_D("Starting to listen...\n");

  // Work loop
  while (1) {
    // Accept a connection.
    connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
      PRINT_I(ERR_ACCEPT_FAIL, strerror(errno));
      continue;
    }

    PRINT_D("Recieved connection. (Connfd %d)\n", connfd);

    syscall_return = pthread_create(&thread, NULL, client_routine,
      (void*)connfd);
  }
}