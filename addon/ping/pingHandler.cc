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

// FIFO 文件路径
static const char* FIFO_PATH = "/tmp/ping_fifo";
// FIFO 文件描述符
static int fifoFd = -1;
// 主循环执行标志
static bool isRunning = true;

// 存储 RTT 的滑动窗口大小
static const size_t WINDOW_SIZE = 10;

// 处理 sigint 信号, Ctrl+C 主循环优雅退出
void
sigintHandler(int)
{
    isRunning = false;
}

// 从 ping 命令的输出正则匹配 RTT 值
// 匹配成功时, 转换匹配数组的第一个元素为 double
// 例 64 bytes from 8.8.8.8: icmp_seq=1 ttl=117 time=12.3 ms -> 返回 12.3
// 匹配失败时, 返回 -1.0
// 例 Request timeout -> 返回 -1.0
double
parse_rtt(const std::string& line)
{
    static const std::regex reg(R"(time=([0-9]+\.?[0-9]*)\s*ms)");
    std::smatch m; // 匹配数组
    if (std::regex_search(line, m, reg) && m.size() == 2)
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

// 计算滑动窗口内RTT的平均值
double
computeRttAvg(const std::deque<double>& window)
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

// 计算 RTT 抖动
double
computeRttJitter(const std::deque<double>& window)
{
    if (window.size() < 2)
    {
        return 0.0;
    }
    double diffSum = 0.0;
    for (size_t i = 1; i < window.size(); ++i)
    {
        diffSum += std::abs(window[i] - window[i - 1]);
    }
    return diffSum / static_cast<double>(window.size() - 1);
}

int
main()
{
    // 注册 sigint 处理函数
    std::signal(SIGINT, sigintHandler);

    if (access(FIFO_PATH, F_OK) == -1)
    {
        // 创建命名管道
        if (mkfifo(FIFO_PATH, 0666) == -1)
        {
            if (errno != EEXIST)
            {
                std::perror("mkfifo");
                return 1;
            }
        }
    }

    // 阻塞等待打开 FIFO 文件
    fifoFd = open(FIFO_PATH, O_RDONLY);
    if (fifoFd < 0)
    {
        std::perror("open FIFO for read");
        return 1;
    }

    std::cout << "[pingHandler] FIFO opened, waiting for data from pingCaller...\n";

    std::deque<double> window;
    constexpr size_t BUF_SIZE = 1024;
    char buf[BUF_SIZE];

    while (isRunning)
    {
        ssize_t n = read(fifoFd, buf, BUF_SIZE - 1); // 读取 FIFO 数据
        if (n > 0)
        { // 分割为单独的行
            buf[n] = '\0';
            std::istringstream iss(buf);
            std::string line;
            while (std::getline(iss, line))
            {
                double rtt = parse_rtt(line);
                if (rtt >= 0.0)
                {
                    // 更新滑动窗口
                    window.push_back(rtt);
                    if (window.size() > WINDOW_SIZE)
                    {
                        window.pop_front();
                    }
                    // 计算并输出统计信息
                    double rttAvg = computeRttAvg(window);
                    double rttJitter = computeRttJitter(window);
                    std::cout << "[pingHandler] New RTT=" << rtt << " ms, " << "Window("
                              << window.size() << "/" << WINDOW_SIZE
                              << ") Recent RTT Avg=" << rttAvg << " ms, "
                              << " RTT Jitter=" << rttJitter << " ms\n";
                }
            }
        }
        else if (n == 0)
        {
            usleep(100000); // 100ms
        }
        else
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::perror("read from FIFO");
            break;
        }
    }

    std::cout << "\n[pingHandler] Received SIGINT, exiting gracefully...\n";
    // 关闭文件描述符
    close(fifoFd);
    fifoFd = -1;
    // 删除 FIFO 文件
    unlink(FIFO_PATH);
    return 0;
}
