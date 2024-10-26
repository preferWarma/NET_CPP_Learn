add_executable(Server_4 tcp_select.cpp)
add_executable(Client_4 client.cpp)

# tcp_lib来自Socket_Encapsulation文件夹的CMakeLists
target_include_directories(tcp_lib PUBLIC ../Socket_Encapsulation)
target_link_libraries(Server_4 PUBLIC tcp_lib)
target_link_libraries(Client_4 PUBLIC tcp_lib)
