#include <arpa/inet.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/_select.h>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_timeval.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

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

    fd_set read_fds; // 需要监视的读事件的socket集合，采用位图表示，大小16字节，1024位，每位代表一个socket
    FD_ZERO(&read_fds);               // 初始化位图，每位都是0
    FD_SET(listen_socket, &read_fds); // 将服务端用于监听的socket添加进位图

    int max_fd = listen_socket;       // 当前位图最大fd值
    timeval timeout;                  // 设置用于超时的结构体
    timeout.tv_sec  = 10;             // s
    timeout.tv_usec = 0;              // us

    while (true) {
        // select函数会修改位图, 所以需要复制一份
        fd_set bitmap = read_fds;

        // 等待事件发生(select调用成功的时候返回已经发生事件的个数)
        int retfds = select(max_fd + 1, &bitmap, nullptr, nullptr, &timeout);
        if (retfds < 0) {
            std::cerr << "error: select failed! " << std::endl;
            close(listen_socket);
            return -1;
        }
        if (retfds == 0) { // 说明select超时了
            std::cerr << "warning: select timeout! " << std::endl;
            continue;
        }
        // 读事件
        for (size_t event_fd = 0; event_fd <= max_fd; ++event_fd) {
            // 如果当前位图中对应的位是0，说明没有事件发生
            if (!FD_ISSET(event_fd, &bitmap)) {
                continue;
            }
            // 对于listen_socket来说, 事件是有新的客户端连上来了
            if (event_fd == listen_socket) {
                sockaddr_in client;
                socklen_t len = sizeof(client);
                int client_fd = accept(listen_socket, (sockaddr*)&client, &len);
                if (client_fd < 0) {
                    std::cerr << "error: accept failed!" << std::endl;
                    continue;
                }
                FD_SET(client_fd, &read_fds);
                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
                PrintTool::print_args("客户端(fd:", client_fd, "ip:", inet_ntoa(client.sin_addr), ")已连接");
            } else { // 接收缓冲区有数据可读或客户端断开连接
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                if (recv(event_fd, buffer, sizeof(buffer), 0) <= 0) { // 客户端断开连接
                    PrintTool::print_args("客户端(fd:", event_fd, ")已断开连接");
                    FD_CLR(event_fd, &read_fds);                      // 从位图中删除
                    close(event_fd);
                    if (event_fd == max_fd) { // 如果是当前位图中最大fd值，需要更新
                        for (size_t i = max_fd - 1; i > 0; --i) {
                            if (FD_ISSET(i, &read_fds)) {
                                max_fd = i;
                                break;
                            }
                        }
                    }
                } else { // 接收缓冲区有数据可读
                    PrintTool::print_args("收到客户端(fd:", event_fd, ")发送的数据:", buffer);
                    // 把收到的报文原封不动的返回
                    send(event_fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    PrintTool::print_args("------------lyf--------------");
    return 0;
}
