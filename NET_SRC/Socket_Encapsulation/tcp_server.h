#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class tcp_server {
private:
    int m_listen_fd;         // 服务端监听的socket
    int m_client_fd;         // 客户端连上来的socket
    std::string m_client_ip; // 客户端的IP地址
    unsigned short m_port;   // 服务端用于通信的端口号

public:
    tcp_server()
        : m_listen_fd(-1), m_client_fd(-1), m_client_ip(""), m_port(0) {}

    ~tcp_server() {
        close_listen();
        close_client();
    }

    // 初始化服务端
    bool
    init(unsigned short port);

    // 受理客户端连接, 从已连接的客户端中取出一个客户端
    bool
    accept();

    // 接受客户端发送的信息
    bool
    receive(std::string& str, size_t maxSize);

    // 接收客户端发送的二进制信息
    bool
    receive(void* data, size_t size);

    // 接收客户端发送的文件信息
    bool
    receive_file(std::string_view filePath, size_t fileSize);

    // 向客户端发送回应信息
    bool
    send(std::string_view message);

    // 向客户端发送二进制信息
    bool
    send(void* data, size_t size);

    // 成员函数的get
    int
    listen_fd() const {
        return m_listen_fd;
    }

    int
    client_fd() const {
        return m_client_fd;
    }

    std::string
    client_ip() const {
        return m_client_ip;
    }

    unsigned short
    port() const {
        return m_port;
    }

    // 关闭监听的socket
    void
    close_listen() {
        if (m_listen_fd != -1) {
            ::close(m_listen_fd);
            m_listen_fd = -1;
        }
    }

    // 关闭客户端的socket
    void
    close_client() {
        if (m_client_fd != -1) {
            ::close(m_client_fd);
            m_client_fd = -1;
        }
    }
};
