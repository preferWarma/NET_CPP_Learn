/*
server.cpp
服务器端程序, 使用我们封装的类简化操作
*/
#include "lyf.h"
#include "stopwatch.h"
#include "tcp_server.h"

using namespace lyf;

int
main(int argc, const char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    assure(argc == 2, "argc != 2");

    tcp_server server;
    if (!server.init(atoi(argv[1]))) {
        std::cerr << "server init failed!" << std::endl;
        return -1;
    }

    if (server.accept()) {
        std::cout << "客户端连接请求已经接受, 客户端IP: " << server.client_ip() << std::endl;
    } else {
        std::cerr << "accept failed!" << std::endl;
    }

    while (true) {
        std::string buffer;
        if (server.receive(buffer, 1024)) {
            std::cout << "接收: " << buffer << std::endl;
        } else {
            break;
        }

        std::string message = "ok";
        if (server.send(message)) {
            std::cout << "发送: " << message << std::endl;
        } else {
            break;
        }
    }

    PrintTool::print_args("-------------lyf--------------");
    return 0;
}
