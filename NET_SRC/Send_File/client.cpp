/*
client.cpp
客户端程序, 使用我们封装的类简化操作
*/
#include "file_status.h"
#include "lyf.h"
#include "stopwatch.h"
#include "tcp_client.h"
#include <iostream>

using namespace lyf;

int
main(int agrc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    assure(agrc == 4, "agrc != 4, \n \
        示例: ./client 192.168.21.103 8080 /Users/liuyifeng/Desktop/code/NET_CPP_Learn/test.cpp");

    tcp_client client;
    if (!client.connect(argv[1], std::stoi(argv[2]))) {
        std::cerr << "error: connect failed!" << std::endl;
        return 0;
    }

    // 以下是发送文件的流程:
    // 1. 发送文件名和文件大小
    auto filestatus = file_status(argv[3]);
    if (!client.send(&filestatus, sizeof(filestatus))) {
        std::cerr << "error: send file status failed!" << std::endl;
        return -1;
    }
    PrintTool::print_args("文件信息已经发送:", filestatus.to_string());

    // 2. 等待服务端的确认报文
    receive_enum ret_enum{receive_enum::Failed};
    if (!client.receive(&ret_enum, sizeof(ret_enum))) {
        std::cerr << "error: receive file status confirm failed!" << std::endl;
        return -1;
    }
    if (ret_enum != receive_enum::Success) {
        std::cerr << "error: file status is not confirmed!" << std::endl;
        return -1;
    }
    PrintTool::print_args("文件信息已经确认成功");

    // 3. 发送文件内容
    if (!client.send_file(filestatus.path(), filestatus.size())) {
        std::cerr << "error: send file content failed!" << std::endl;
        return -1;
    }
    PrintTool::print_args("文件内容发送完成");

    // 4. 接收服务端的确认报文, 表示文件传输完成
    ret_enum = receive_enum::Failed;
    if (!client.receive(&ret_enum, sizeof(ret_enum))) {
        std::cerr << "error: receive over confirm failed!" << std::endl;
        return -1;
    }
    if (ret_enum != receive_enum::Success) {
        std::cerr << "error: over is not confirmed!" << std::endl;
        return -1;
    }
    PrintTool::print_args("文件传输完成");

    PrintTool::print_args("------------lyf--------------");
    return 0;
}
