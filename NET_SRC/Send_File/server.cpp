/*
server.cpp
服务器端程序(多线程版) 实现文本文件和二进制文件传输功能
父进程只负责处理客户端的连接请求, 父进程只管理用于监听的socket
子进程只负责与客户端通信, 子进程只管理用于通讯的socket

Linux命令行提示:
使用 ps -ef | grep [process_name] 找到进程的pid
mac 下使用losf -p [pid] 来查看进程打开的文件
Ubuntu 下使用 ls /proc/[pid]/fd 来查看进程打开的文件
*/
#include "file_status.h"
#include "lyf.h"
#include "stopwatch.h"
#include "tcp_server.h"
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

using namespace lyf;

tcp_server server; // 定义为全局变量, 在子进程和父进程终止时可以访问

void
FatherExit(int sig) {
    // 防止信号处理函数在执行的过程中再次被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    PrintTool::print_args("父进程(pid =", getpid(), ")已终止");
    // 通知子进程结束
    kill(0, SIGTERM);
    // 释放父进程持有的资源
    server.close_listen();
    // 结束父进程
    exit(0);
}

void
ChildExit(int sig) {
    // 防止信号处理函数在执行的过程中再次被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    PrintTool::print_args("\t子进程(pid =", getpid(), ")已终止");
    // 释放子进程持有的资源
    server.close_client();
    // 结束子进程
    exit(0);
}

std::string
newFilePath(const std::string& floder, const std::string& filename) {
    using namespace std::filesystem;
    std::string path = floder + "/" + filename;
    size_t idx       = 1;

    while (exists(path)) {
        path = floder + "/" + filename + "_" + std::to_string(idx++);
    }
    return path;
}

int
main(int argc, const char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    assure(argc == 3, "argc != 3, 示例: ./Server 8080 ./tmp");

    // 忽略其他信号, 避免被打扰
    for (size_t i = 1; i <= 64; ++i) {
        signal(i, SIG_IGN);
    }
    // 设置父进程的结束信号
    signal(SIGINT, FatherExit);
    signal(SIGTERM, FatherExit);

    if (!server.init(atoi(argv[1]))) {
        std::cerr << "server init failed!" << std::endl;
        return -1;
    }

    while (true) {
        if (server.accept()) {
            std::cout << "客户端连接请求已经接受, 客户端IP: " << server.client_ip() << std::endl;
        } else {
            std::cerr << "accept failed!" << std::endl;
            return -1;
        }

        // 创建新的进程
        // pid > 0 表示此时是父进程在操作, 创建了一个子进程
        // pid == 0 表示此时是子进程在操作
        // pid == -1 表示父进程创建子进程失败
        int pid = fork();
        if (pid == -1) { // 子进程创建失败, 系统资源不足
            std::cerr << "error: fork failed!" << std::endl;
            return -1;
        } else if (pid > 0) {      // 子进程创建成功, 父进程回到循环开始, 继续等待连接
            server.close_client(); // 父进程只管受理连接, 不管通信
            continue;
        }

        // 子进程用于接收消息和发送消息, 不管连接
        server.close_listen();
        signal(SIGTERM, ChildExit); // 设置子进程的结束信号
        signal(SIGINT, SIG_IGN);    // 子进程无需捕获SIGINT信号

        // 以下是接收文件的流程
        // 1. 接收文件名和文件大小信息
        file_status filestatus;
        if (!server.receive(&filestatus, sizeof(filestatus))) {
            std::cerr << "error: receive file status failed!" << std::endl;
            return -1;
        }
        cout << "文件信息已经接收:" << filestatus.to_string() << endl;

        // 2. 给客户端发送确认报文, 表示客户端可以发送文件了
        if (!server.send("ok")) {
            std::cerr << "error: send file status confirm failed!" << std::endl;
            return -1;
        }
        cout << "文件信息确认报文已经发送" << endl;

        // 3. 接受文件内容
        std::string fileSavePath = newFilePath(argv[2], filestatus.filename());
        if (!server.receive_file(fileSavePath, filestatus.size())) {
            std::cerr << "error: receive file content failed!" << std::endl;
            return -1;
        }
        cout << "文件内容接收完成" << endl;

        // 4. 给客户端发送确认报文, 表示文件接收完成
        if (!server.send("ok")) {
            std::cerr << "error: send over confirm failed!" << std::endl;
        }
        cout << "收到的文件保存在: " << fileSavePath << endl;
        ;

        // 子进程处通信理完成后需要退出, 否则子进程会回到循环开始执行accpet函数
        break;
    }

    PrintTool::print_args("-------------lyf--------------");
    return 0;
}