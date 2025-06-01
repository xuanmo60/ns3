#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/ping_fifo"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <IP> <count>\n";
    return 1;
  }
  const char *ip = argv[1];
  int count = std::atoi(argv[2]);
  if (count <= 0) {
    std::cerr << "Invalid count value\n";
    return 1;
  }
  mkfifo(FIFO_NAME, 0666);
  pid_t pid = fork();
  if (pid == 0) {
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd < 0) {
      perror("open");
      exit(1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    execlp("ping", "ping", "-c", std::to_string(count).c_str(), ip, nullptr);
    perror("execlp");
    exit(1);
  } else if (pid > 0) {
    wait(nullptr);
  } else {
    perror("fork");
    return 1;
  }
  return 0;
}
