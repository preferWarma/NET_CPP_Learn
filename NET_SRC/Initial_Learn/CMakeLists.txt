add_executable(Sever server.cpp)
add_executable(Client client.cpp)

target_include_directories(Sever PUBLIC ../../include)
target_include_directories(Client PUBLIC ../../include)