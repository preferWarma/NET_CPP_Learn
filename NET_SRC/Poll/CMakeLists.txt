add_executable(Server_5 tcp_poll.cpp)
add_executable(Client_5 client.cpp)

# tcp_lib来自Socket_Encapsulation文件夹的CMakeLists
target_include_directories(tcp_lib PUBLIC ../Socket_Encapsulation)
target_link_libraries(Server_5 PUBLIC tcp_lib)
target_link_libraries(Client_5 PUBLIC tcp_lib)
