#include <arpa/inet.h>
#include <ctime>
#include <iostream>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_timeval.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__linux__)
#include <sys/epoll.h>
#elif defined(__MACH__)
#include <sys/event.h>
#endif

#include "lyf.h"
#include "stopwatch.h"

using namespace lyf;

// 初始化服务端, 初始化成功返回用于监听的套接字, 失败返回-1
int
initServer(int port) {
    // 创建用于监听的socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        std::cerr << "error: create socket failed!" << std::endl;
        return -1;
    }

    // 设置端口复用
    int reuse = 1;
    auto len  = sizeof(reuse);
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, len) == -1) {
        std::cerr << "error: setsockopt failed!" << std::endl;
        close(listen_socket);
        return -1;
    }

    // 绑定端口和地址
    sockaddr_in sever_addr;
    sever_addr.sin_family      = AF_INET;           // 指定协议
    sever_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 服务端任意IP都可以访问
    sever_addr.sin_port        = htons(port);       // 指定端口
    if (bind(listen_socket, (sockaddr*)&sever_addr, sizeof(sever_addr)) == -1) {
        std::cerr << "error: bind failed!" << std::endl;
        close(listen_socket);
        return -1;
    }
    // socket设置为监听模式
    if (listen(listen_socket, 5) == -1) {
        std::cerr << "error: listen failed!" << std::endl;
        close(listen_socket);
        return -1;
    }
    return listen_socket;
}

int
main(int argc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    assure(argc == 2, "argc != 2. 示例: ./Server 8080\n");

    int port = atoi(argv[1]);
    assert(port > 1024);
    int listen_socket = initServer(port);
    if (listen_socket == -1) {
        std::cerr << "error: init server failed!" << std::endl;
        return -1;
    } else {
        std::cout << "服务器初始化成功, 当前监听socket号:" << listen_socket << " 端口号: " << port << std::endl;
    }

    // 读事件: 1) 已连接队列中有已经准备好的socket（有新的客户端连上来了）。
    //        2) 接收缓存中有数据可以读（对端发送的报文已到达）。
    //        3) tcp连接已断开（对端调用close()函数关闭了连接）。
    // 写事件: 发送缓冲区没有满，可以写入数据（可以向对端发送报文）。

#if defined(__linux__)
    int epoll_fd = epoll_create(1); // 创建epoll句柄
    if (epoll_fd == -1) {
        std::cerr << "error: epoll_create failed!" << std::endl;
        close(listen_socket);
        return -1;
    }
    epoll_event ev;             // epoll事件用到的结构体
    ev.data.fd = listen_socket; // 指定事件的自定义数据, 会随着epoll_wait()返回的事件一起传递
    ev.events  = EPOLLIN;       // 监听读事件
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &ev); // 注册监听事件
    epoll_event ret_evs[10];                                // epoll返回的事件集合

    constexpr size_t timeout_ms = 10'000;                   // 设置超时时间为10s

    while (true) {
        // 等待事件发生(epoll_wait调用成功的时候返回触发的事件数量，即events数组中填充的事件数量)
        int retfds = epoll_wait(epoll_fd, ret_evs, 10, timeout_ms);

        if (retfds < 0) {
            std::cerr << "error: epoll failed! " << std::endl;
            close(listen_socket);
            return -1;
        }
        if (retfds == 0) { // 说明epoll超时了
            std::cerr << "warning: epoll timeout! " << std::endl;
            continue;
        }
        // 读事件
        for (size_t i = 0; i < retfds; ++i) {
            // 对于listen_socket来说, 事件是有新的客户端连上来了
            if (ret_evs[i].data.fd == listen_socket) {
                sockaddr_in client;
                socklen_t len = sizeof(client);
                int client_fd = accept(listen_socket, (sockaddr*)&client, &len);
                if (client_fd < 0) {
                    std::cerr << "error: accept failed!" << std::endl;
                    continue;
                }
                // 将client_fd注册到epoll中
                ev.data.fd = listen_socket; // 指定事件的自定义数据, 会随着epoll_wait()返回的事件一起传递
                ev.events = EPOLLIN;        // 监听读事件
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &ev); // 注册监听事件

                PrintTool::print_args("客户端(fd:", client_fd, "ip:", inet_ntoa(client.sin_addr), ")已连接");
            } else { // 接收缓冲区有数据可读或客户端断开连接
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                if (recv(ret_evs[i].data.fd, buffer, sizeof(buffer), 0) <= 0) { // 客户端断开连接
                    PrintTool::print_args("客户端(fd:", ret_evs[i].data.fd, ")已断开连接");
                    close(ret_evs[i].data.fd);
                    // 从epollfd中删除客户端的socket，如果socket被关闭了，会自动从epollfd中删除
                } else { // 接收缓冲区有数据可读
                    PrintTool::print_args("收到客户端(fd:", ret_evs[i].data.fd, ")发送的数据:", buffer);
                    // 把收到的报文原封不动的返回
                    send(ret_evs[i].data.fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
#elif defined(__MACH__)
    int kqueue_fd = kqueue();                                     // 创建kqueue句柄
    struct kevent kev;
    EV_SET(&kev, listen_socket, EVFILT_READ, EV_ADD, 0, 0, NULL); // 为listen_socket准备读事件
    if (kevent(kqueue_fd, &kev, 1, NULL, 0, NULL) == -1) {
        std::cerr << "error: kevent failed!" << std::endl;
        close(listen_socket);
        return -1;
    }

    struct kevent ret_evs[10];               // kqueue返回的事件集合
    constexpr timespec timeout_ms = {10, 0}; // 设置超时时间为10s

    while (true) {
        // 等待事件发生
        int retfds = kevent(kqueue_fd, NULL, 0, ret_evs, 10, &timeout_ms);

        if (retfds < 0) {
            std::cerr << "error: kqueue failed! " << std::endl;
            return -1;
        }
        if (retfds == 0) { // 说明kqueue超时了
            std::cerr << "warning: kqueue timeout! " << std::endl;
            continue;
        }
        // 读事件
        for (size_t i = 0; i < retfds; ++i) {
            // 对于listen_socket来说, 事件是有新的客户端连上来了
            if (ret_evs[i].ident == listen_socket) {
                sockaddr_in client;
                socklen_t len = sizeof(client);
                int client_fd = accept(listen_socket, (sockaddr*)&client, &len);
                if (client_fd < 0) {
                    std::cerr << "error: accept failed!" << std::endl;
                    continue;
                }
                // 将client_fd注册到kqueue中
                EV_SET(&ret_evs[i], client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kqueue_fd, &ret_evs[i], 1, NULL, 0, NULL) == -1) {
                    std::cerr << "warning: kevent failed!" << std::endl;
                    close(client_fd);
                    continue;
                }

                PrintTool::print_args("客户端(fd:", client_fd, "ip:", inet_ntoa(client.sin_addr), ")已连接");
            } else { // 接收缓冲区有数据可读或客户端断开连接
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                if (recv(ret_evs[i].ident, buffer, sizeof(buffer), 0) <= 0) { // 客户端断开连接
                    PrintTool::print_args("客户端(fd:", ret_evs[i].ident, ")已断开连接");
                    close(ret_evs[i].ident);
                    // 从kqueue中删除客户端的socket，如果socket被关闭了，会自动从kqueue中删除
                } else { // 接收缓冲区有数据可读
                    PrintTool::print_args("收到客户端(fd:", ret_evs[i].ident, ")发送的数据:", buffer);
                    // 把收到的报文原封不动的返回
                    send(ret_evs[i].ident, buffer, strlen(buffer), 0);
                }
            }
        }
    }
#endif

    PrintTool::print_args("------------lyf--------------");
    return 0;
}
