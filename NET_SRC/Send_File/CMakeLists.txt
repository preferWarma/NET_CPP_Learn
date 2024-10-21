add_executable(Server_3 server.cpp)
add_executable(Client_3 client.cpp)

# tcp_lib来自Socket_Encapsulation文件夹的CMakeLists
target_include_directories(tcp_lib PUBLIC ../Socket_Encapsulation)
target_link_libraries(Server_3 PUBLIC tcp_lib)
target_link_libraries(Client_3 PUBLIC tcp_lib)
