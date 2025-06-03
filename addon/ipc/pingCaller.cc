// pingCaller.cc
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static const char* FIFO_PATH = "/tmp/ping_fifo";
static int fifo_fd = -1;
static FILE* ping_pipe = nullptr;

// SIGINT handler: close ping subprocess and FIFO, then exit
void
handle_sigint(int)
{
    if (ping_pipe)
    {
        pclose(ping_pipe);
        ping_pipe = nullptr;
    }
    if (fifo_fd >= 0)
    {
        close(fifo_fd);
        fifo_fd = -1;
    }
    std::cout << "\n[pingCaller] Received SIGINT, exiting gracefully...\n";
    std::exit(0);
}

int
main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ping arguments>\n"
                  << "Example: " << argv[0] << " -c 100 google.com\n";
        return 1;
    }

    // Register SIGINT handler
    std::signal(SIGINT, handle_sigint);

    // Create FIFO if it does not exist
    if (access(FIFO_PATH, F_OK) == -1)
    {
        if (mkfifo(FIFO_PATH, 0666) == -1)
        {
            if (errno != EEXIST)
            {
                std::perror("mkfifo");
                return 1;
            }
        }
    }

    // Open FIFO for writing (blocks until a reader opens it)
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd < 0)
    {
        std::perror("open FIFO for write");
        return 1;
    }

    // Build the ping command
    std::ostringstream cmd;
    cmd << "ping";
    for (int i = 1; i < argc; ++i)
    {
        cmd << " " << argv[i];
    }
    std::string ping_cmd = cmd.str();
    std::cout << "[pingCaller] Executing command: " << ping_cmd << "\n";

    // Start ping using popen
    ping_pipe = popen(ping_cmd.c_str(), "r");
    if (!ping_pipe)
    {
        std::perror("popen");
        close(fifo_fd);
        return 1;
    }

    // Read ping output line by line and write to FIFO
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), ping_pipe))
    {
        size_t len = std::strlen(buffer);
        // Write the raw output (including newline) to FIFO
        ssize_t w = write(fifo_fd, buffer, len);
        if (w < 0)
        {
            std::perror("write to FIFO");
            break;
        }
    }

    // Close ping subprocess and FIFO when done
    pclose(ping_pipe);
    ping_pipe = nullptr;
    close(fifo_fd);
    fifo_fd = -1;
    std::cout << "[pingCaller] Ping process ended, exiting.\n";
    return 0;
}
