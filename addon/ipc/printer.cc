#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/ping_fifo"

volatile sig_atomic_t stop = 0;

void signal_handler(int signum) { stop = 1; }

int main() {
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, NULL) == -1 ||
      sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction");
    return 1;
  }
  std::cout << "Printer started. PID: " << getpid() << "\n";
  std::cout << "Press Ctrl+C to exit.\n";
  if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
    perror("mkfifo");
    return 1;
  }
  while (!stop) {
    int fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
      if (stop)
        break;
      perror("open");
      sleep(1);
      continue;
    }
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    std::cout << "Waiting for ping data...\n";
    int ret = poll(&pfd, 1, -1);
    if (ret == -1) {
      if (errno == EINTR) {
        close(fd);
        if (stop)
          break;
        continue;
      }
      perror("poll");
      close(fd);
      sleep(1);
      continue;
    }
    if (pfd.revents & POLLIN) {
      std::cout << "\n=== New ping results ===\n";
      char buffer[1024];
      ssize_t bytes_read;
      int flags = fcntl(fd, F_GETFL, 0);
      fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
      while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        std::cout << buffer;
      }
      fcntl(fd, F_SETFL, flags);
      std::cout << "=== End of results ===\n";
    }

    close(fd);
  }
  unlink(FIFO_NAME);
  std::cout << "\nPrinter exited gracefully.\n";
  return 0;
}
