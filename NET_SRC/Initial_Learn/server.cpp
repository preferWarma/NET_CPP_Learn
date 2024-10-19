/*
server.cpp
服务器端程序
*/
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "lyf.h"
#include "stopwatch.h"

using namespace lyf;

int
main(int argc, const char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);

    assure(argc == 2, "argc != 2");

    // 1. 创建服务端的socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0); // 服务端用来监听的socket
    if (listenfd == -1) {
        std::cerr << "socket failed!" << std::endl;
        return -1;
    }

    // 2. 把服务端用于通信的IP和端口绑定到socket上
    sockaddr_in sever_addr;
    memset(&sever_addr, 0, sizeof(sever_addr));
    sever_addr.sin_family      = AF_INET;                        // 指定协议
    sever_addr.sin_addr.s_addr = htonl(INADDR_ANY);              // 服务端任意网卡到IP都可以用于通信
    sever_addr.sin_port        = htons((uint16_t)atoi(argv[1])); // 指定端口号
    // 绑定服务端的IP和端口
    if (bind(listenfd, (sockaddr*)&sever_addr, sizeof(sever_addr)) != 0) {
        std::cerr << "bind failed!" << std::endl;
        close(listenfd);
        return -1;
    }

    // 3. 将服务端socket设置为可监听状态
    if ((listen(listenfd, 5)) != 0) {
        std::cerr << "listen failed!" << std::endl;
        close(listenfd);
        return -1;
    }

    // 4. 受理客户端的连接请求，如果没有连接请求，则accept函数则会阻塞等待
    int clientfd = accept(listenfd, 0, 0); // clientfd为客户端连接上来的socket
    if (clientfd == -1) {
        std::cerr << "accept failed!" << std::endl;
        close(listenfd);
        return -1;
    }
    std::cout << "客户端连接请求已经接受" << std::endl;

    // 5. 与客户端通信，接收客户端发来的信息，返回信息OK
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        // 如果客户端没有发送报文, 则陷入阻塞
        // 如果客户端断开连接, 则返回0
        if ((recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) {
            break;
        }
        std::cout << "接收: " << buffer << std::endl;

        // 生成回应报文
        strcpy(buffer, "ok");
        // 向客户端发送回应报文
        if ((send(clientfd, buffer, strlen(buffer), 0)) <= 0) {
            std::cerr << "send failed!" << std::endl;
            break;
        }
        std::cout << "发送: " << buffer << std::endl;
    }

    // 6. 关闭资源
    close(listenfd);
    close(clientfd);

    PrintTool::print_args("-------------lyf--------------");
    return 0;
}
