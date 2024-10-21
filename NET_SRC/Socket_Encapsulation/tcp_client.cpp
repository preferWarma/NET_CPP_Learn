#include "tcp_client.h"
#include <fstream>
#include <ios>
#include <iostream>

bool
tcp_client::connect(std::string_view ip, unsigned short port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 服务器端端socket
    hostent* h = gethostbyname(ip.data());        // 用于存放服务端IP的结构体
    if (sockfd == -1 || h == nullptr) {
        if (sockfd != -1) {
            ::close(sockfd);
        }
        return false;
    }

    sockaddr_in server_addr; // 用于存放服务端IP和端口的结构体。
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;                              // 指定协议
    memcpy(&server_addr.sin_addr, h->h_addr, (size_t)h->h_length); // 指定服务端的IP地址。
    server_addr.sin_port = htons(port);                            // 指定服务端的通信端口。

    // 向服务器发起连接请求
    if (::connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        ::close(sockfd);
        return false;
    }

    // 此时已经连接成功了, 对成员变量进行赋值
    m_fd   = sockfd;
    m_ip   = ip;
    m_port = port;
    return true;
}

bool
tcp_client::send(std::string_view message) {
    // 发送请求报文
    if (::send(m_fd, message.data(), message.size(), 0) <= 0) {
        return false;
    }
    return true;
}

bool
tcp_client::send(void* data, size_t size) {
    // 发送请求报文
    if (::send(m_fd, data, size, 0) <= 0) {
        return false;
    }
    return true;
}

bool
tcp_client::send_file(std::string_view filePath, size_t fileSize) {
    std::ifstream fin(filePath, std::ios::binary);
    if (!fin.is_open()) {
        return false;
    }
    long readn;               // 每次循环读取字节数
    long cur_total_bytes = 0; // 当前总读取字节数
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        readn = (fileSize - cur_total_bytes >= 1024) ? 1024 : fileSize - cur_total_bytes;
        fin.read(buffer, readn);
        if (!this->send(buffer, readn)) {
            return false;
        }
        cur_total_bytes += readn;
        std::cout << "readn: " << readn << std::endl;
        if (cur_total_bytes == fileSize) {
            break;
        }
    }
    return true;
}

bool
tcp_client::receive(std::string& buffer, size_t maxSize) {
    buffer.clear();
    buffer.resize(maxSize);
    // 接收回应报文
    long readn = ::recv(m_fd, &buffer[0], buffer.size(), 0);
    if (readn <= 0) {
        buffer.clear();
        return false;
    }
    buffer.resize(readn);
    return true;
}

bool
tcp_client::receive(void* data, size_t size) {
    if (recv(m_fd, data, size, 0) <= 0) {
        return false;
    }
    return true;
}
