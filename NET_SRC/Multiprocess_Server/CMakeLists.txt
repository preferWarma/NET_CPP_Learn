add_executable(Server_2 server.cpp)
add_executable(Client_2 client.cpp)

# tcp_lib来自Socket_Encapsulation文件夹的CMakeLists
target_include_directories(tcp_lib PUBLIC ../Socket_Encapsulation)
target_link_libraries(Server_2 PUBLIC tcp_lib)
target_link_libraries(Client_2 PUBLIC tcp_lib)
