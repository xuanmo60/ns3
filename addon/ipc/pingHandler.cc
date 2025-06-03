// pingHandler.cc
#include <cerrno>
#include <csignal>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static const char* FIFO_PATH = "/tmp/ping_fifo";
static int fifo_fd = -1;
static bool keep_running = true;

// Window size: number of recent RTT samples to keep
static const size_t WINDOW_SIZE = 10;

// SIGINT handler: set flag to exit the main loop
void
handle_sigint(int)
{
    keep_running = false;
}

// Parse RTT value (in ms) from a single ping output line.
// If no valid RTT is found, return a negative value.
double
parse_rtt(const std::string& line)
{
    // Typical Linux ping output contains "time=12.3 ms"
    static const std::regex re_time(R"(time=([0-9]+\.?[0-9]*)\s*ms)");
    std::smatch m;
    if (std::regex_search(line, m, re_time) && m.size() == 2)
    {
        try
        {
            return std::stod(m[1].str());
        }
        catch (...)
        {
            return -1.0;
        }
    }
    return -1.0;
}

// Compute average RTT from the window
double
compute_avg(const std::deque<double>& window)
{
    if (window.empty())
    {
        return 0.0;
    }
    double sum = 0.0;
    for (double v : window)
    {
        sum += v;
    }
    return sum / static_cast<double>(window.size());
}

// Compute RTT jitter (average absolute difference between adjacent samples)
double
compute_jitter(const std::deque<double>& window)
{
    if (window.size() < 2)
    {
        return 0.0;
    }
    double sum_diff = 0.0;
    for (size_t i = 1; i < window.size(); ++i)
    {
        sum_diff += std::abs(window[i] - window[i - 1]);
    }
    return sum_diff / static_cast<double>(window.size() - 1);
}

int
main()
{
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

    // Open FIFO for reading (blocks until a writer opens it)
    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd < 0)
    {
        std::perror("open FIFO for read");
        return 1;
    }

    std::cout << "[pingHandler] FIFO opened, waiting for data from pingCaller...\n";

    std::deque<double> window;
    constexpr size_t BUF_SIZE = 1024;
    char buf[BUF_SIZE];

    // Main loop: read from FIFO and process lines
    while (keep_running)
    {
        ssize_t n = read(fifo_fd, buf, BUF_SIZE - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            std::istringstream iss(buf);
            std::string line;
            // Split by lines, since one read may contain multiple lines
            while (std::getline(iss, line))
            {
                double rtt = parse_rtt(line);
                if (rtt >= 0.0)
                {
                    // Update sliding window
                    window.push_back(rtt);
                    if (window.size() > WINDOW_SIZE)
                    {
                        window.pop_front();
                    }
                    // Compute metrics
                    double avg_rtt = compute_avg(window);
                    double jitter = compute_jitter(window);
                    // Print results
                    std::cout << "[pingHandler] New RTT=" << rtt << " ms, " << "Window("
                              << window.size() << "/" << WINDOW_SIZE << ") Avg RTT=" << avg_rtt
                              << " ms, " << "Recent RTT Jitter=" << jitter << " ms\n";
                }
                // If no RTT was parsed, ignore the line
            }
        }
        else if (n == 0)
        {
            // Write end closed; sleep briefly then continue
            usleep(100000); // 100 ms
        }
        else
        {
            if (errno == EINTR)
            {
                // Interrupted by signal; check keep_running
                continue;
            }
            std::perror("read from FIFO");
            break;
        }
    }

    // Cleanup before exiting
    std::cout << "\n[pingHandler] Received SIGINT, exiting gracefully...\n";
    close(fifo_fd);
    fifo_fd = -1;
    // Optionally remove the FIFO file
    unlink(FIFO_PATH);

    return 0;
}
