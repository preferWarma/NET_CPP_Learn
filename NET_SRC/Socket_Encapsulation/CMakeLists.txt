set(src_tcp tcp_client.cpp tcp_server.cpp)
add_library(tcp_lib OBJECT ${src_tcp}) # 设置库名以及库对应的cpp文件

add_executable(Server_1 server.cpp)
add_executable(Client_1 client.cpp)

target_include_directories(tcp_lib PUBLIC ../../include)

target_link_libraries(Server_1 PUBLIC tcp_lib)
target_link_libraries(Client_1 PUBLIC tcp_lib)