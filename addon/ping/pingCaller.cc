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

// FIFO 文件路径
static const char* FIFO_PATH = "/tmp/ping_fifo";
// FIFO 文件描述符
static int fifoFd = -1;
// ping 进程的管道指针
static FILE* pingPipe = nullptr;

// 信号处理函数
void
sigintHandler(int)
{
    if (pingPipe)
    {
        // 关闭 ping 进程
        pclose(pingPipe);
        pingPipe = nullptr;
    }
    if (fifoFd >= 0)
    {
        // 关闭 FIFO 文件
        close(fifoFd);
        fifoFd = -1;
    }
    std::cout << "\n[pingCaller] Received SIGINT, exiting gracefully...\n";
    std::exit(0); // 退出
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

    std::signal(SIGINT, sigintHandler);

    // 如果不存在 FIFO 文件, 则创建
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

    // 阻塞等待打开 FIFO 文件
    fifoFd = open(FIFO_PATH, O_WRONLY);
    if (fifoFd < 0)
    {
        std::perror("open FIFO for write");
        return 1;
    }

    // 构建 ping 命令
    std::ostringstream cmd;
    cmd << "ping";
    for (int i = 1; i < argc; ++i)
    {
        cmd << " " << argv[i];
    }
    std::string pingCmd = cmd.str();
    std::cout << "[pingCaller] Executing command: " << pingCmd << "\n";

    // 执行 ping 命令
    // r, read 模式: 打开管道, 读取命令输出
    pingPipe = popen(pingCmd.c_str(), "r");
    if (!pingPipe)
    {
        std::perror("popen");
        close(fifoFd);
        return 1;
    }

    char buf[1024];
    while (fgets(buf, sizeof(buf), pingPipe))
    {
        size_t len = std::strlen(buf);
        ssize_t w = write(fifoFd, buf, len); // 写入 FIFO 文件
        if (w < 0)
        {
            std::perror("write to FIFO");
            break;
        }
    }

    pclose(pingPipe); // 关闭 ping 进程
    pingPipe = nullptr;
    close(fifoFd); // 关闭 FIFO 文件
    fifoFd = -1;
    std::cout << "[pingCaller] Ping process ended, exiting.\n";
    return 0;
}
