#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
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
main(int agrc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);

    assure(agrc == 3, "agrc != 3");

    // 1. 创建客户端socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "socket error" << std::endl;
        return -1;
    }

    // 2. 向服务器发起连接请求
    hostent* h = gethostbyname(argv[1]); // 用于存放服务端IP的结构体
    if (h == 0) {
        std::cerr << "get host failed" << std::endl;
        close(sockfd);
        return -1;
    }
    struct sockaddr_in server_addr; // 用于存放服务端IP和端口的结构体。
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr, h->h_addr, (size_t)h->h_length); // 指定服务端的IP地址。
    server_addr.sin_port = htons((uint16_t)atoi(argv[2]));         // 指定服务端的通信端口。
    // 向服务器发起连接请求
    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "connect error" << std::endl;
        close(sockfd);
        return -1;
    }

    // 3. 连接成功，与服务端通信，向服务端发送一个报文后等待回复，然后发送下一个报文
    char buffer[1024];
    constexpr size_t ChatSize = 3; // 通讯次数
    for (size_t i = 0; i < ChatSize; ++i) {
        long i_ret;
        std::snprintf(buffer, sizeof(buffer), "这是第条%zu信息, 编号为%zu:", i, i);
        // 发送请求报文
        if ((i_ret = send(sockfd, buffer, strlen(buffer), 0)) <= 0) {
            std::cerr << "send failed!" << std::endl;
            break;
        }
        std::cout << "发送" << buffer << std::endl;
        memset(buffer, 0, sizeof(buffer));

        // 接受服务端的回应报文, 如果没有收到则阻塞等待
        if ((i_ret = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
            std::cerr << "recv failed" << std::endl;
            break;
        }
        std::cout << "收到" << buffer << std::endl;
        sleep(1);
    }

    // 4. 关闭连接
    close(sockfd);

    PrintTool::print_args("------------lyf--------------");
    return 0;
}
