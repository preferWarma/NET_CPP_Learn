/*
client.cpp
客户端程序, 使用我们封装的类简化操作
*/
#include "lyf.h"
#include "stopwatch.h"
#include "tcp_client.h"
#include <iostream>
#include <string>

using namespace lyf;

int
main(int agrc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    assure(agrc == 3, "agrc != 3");

    tcp_client client;
    if (!client.connect(argv[1], std::stoi(argv[2]))) {
        std::cerr << "error: connect failed!" << std::endl;
        return 0;
    }

    constexpr size_t ChatSize = 3; // 通讯次数
    for (size_t i = 0; i < ChatSize; ++i) {
        std::string message = "这是第条" + std::to_string(i) + "信息";
        // 发送请求报文
        if (!client.send(message)) {
            std::cerr << "send failed!" << std::endl;
            break;
        }
        std::cout << "发送: " << message << std::endl;

        // 接受服务端的回应报文, 如果没有收到则阻塞等待
        string buffer;
        if (!client.receive(buffer, 1024)) {
            std::cerr << "recv failed" << std::endl;
            break;
        }
        std::cout << "收到: " << buffer << std::endl;

        sleep(1); // 模拟客户端与服务端的交互时间间隔
    }

    PrintTool::print_args("------------lyf--------------");
    return 0;
}
