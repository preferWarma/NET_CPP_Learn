#include "tcp_server.h"
#include <fstream>
#include <ios>
#include <iostream>

bool
tcp_server::init(unsigned short port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;           // 指定协议
    server_addr.sin_port        = htons(port);       // 指定端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 指定IP

    // 绑定服务端的端口和IP
    if (bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        ::close(listen_fd);
        return false;
    }

    // 设置为可监听状态
    if (::listen(listen_fd, 5) == -1) {
        ::close(listen_fd);
        return false;
    }

    // 初始化成功, 为成员变量赋值
    m_listen_fd = listen_fd;
    m_port      = port;
    return true;
}

bool
tcp_server::accept() {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd             = ::accept(m_listen_fd, (sockaddr*)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        return false;
    }
    // 接受连接成功, 为成员变量赋值
    m_client_fd = client_fd;
    m_client_ip = inet_ntoa(client_addr.sin_addr);
    return true;
}

bool
tcp_server::receive(std::string& str, size_t maxSize) {
    str.clear();
    str.resize(maxSize);
    long readn;
    if ((readn = recv(m_client_fd, &str[0], str.size(), 0)) <= 0) {
        return false;
    }
    str.resize(readn);
    return true;
}

bool
tcp_server::receive(void* data, size_t size) {
    if (recv(m_client_fd, data, size, 0) <= 0) {
        return false;
    }
    return true;
}

bool
tcp_server::receive_file(std::string_view filePath, size_t fileSize) {
    std::ofstream fout(filePath.data(), std::ios::binary);
    if (!fout.is_open()) {
        return false;
    }
    long writen;              // 每次循环读取字节数
    long cur_total_bytes = 0; // 当前总读取字节数
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        writen = (fileSize - cur_total_bytes >= 1024) ? 1024 : fileSize - cur_total_bytes;
        if (!this->receive(buffer, writen)) {
            return false;
        }
        fout.write(buffer, writen);
        cur_total_bytes += writen;
        if (cur_total_bytes == fileSize) {
            break;
        }
    }
    return true;
}

bool
tcp_server::send(std::string_view message) {
    if ((::send(m_client_fd, message.data(), message.size(), 0)) <= 0) {
        return false;
    }
    return true;
}

bool
tcp_server::send(void* data, size_t size) {
    if ((::send(m_client_fd, data, size, 0)) <= 0) {
        return false;
    }
    return true;
}
