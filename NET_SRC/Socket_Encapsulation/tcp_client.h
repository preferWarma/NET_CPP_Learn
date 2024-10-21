/*
tcp_client.h
TCP通信的客户端类
*/
#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <netdb.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class tcp_client {
private:
    int m_fd;              // 客户端socket标识符, -1表示未连接或连接已经断开
    std::string m_ip;      // 服务端IP地址或域名
    unsigned short m_port; // 16位通讯端口号

public:
    inline tcp_client()
        : m_fd(-1), m_ip(""), m_port(0) {}

    inline ~tcp_client() {
        this->close();
    }

    // 向服务器端发送连接请求, 连接成功返回true, 失败返回flase
    bool
    connect(std::string_view ip, unsigned short port);

    // 向服务器端发送信息
    bool
    send(std::string_view message);

    // 向服务器端发送二进制信息
    bool
    send(void* data, size_t size);

    // 向服务器端发送文件信息
    bool
    send_file(std::string_view filePath, size_t fileSize);

    // 接收服务器端发送的回应信息
    bool
    receive(std::string& str, size_t maxSize);

    // 接收服务器端发送的二进制信息
    bool
    receive(void* data, size_t size);

    // 若socket处于连接状态, 则释放socket资源
    void
    close() {
        if (hasConnected()) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    // 成员变量的get函数
    int
    fd() const {
        return m_fd;
    }

    std::string
    ip() const {
        return m_ip;
    }

    unsigned short
    port() const {
        return m_port;
    }

    // 当前客户端是否已连接, 已经连接成功则返回true
    bool
    hasConnected() const {
        return m_fd != -1;
    }
};

#endif // !TCP_CLIENT_H_
