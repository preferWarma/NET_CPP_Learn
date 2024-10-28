add_executable(Server_6 tcp_epoll.cpp)
add_executable(Client_6 client.cpp)

# tcp_lib来自Socket_Encapsulation文件夹的CMakeLists
target_include_directories(tcp_lib PUBLIC ../Socket_Encapsulation)
target_link_libraries(Server_6 PUBLIC tcp_lib)
target_link_libraries(Client_6 PUBLIC tcp_lib)