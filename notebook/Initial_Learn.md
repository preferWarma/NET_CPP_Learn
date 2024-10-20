# 第一个网络通讯程序

## 一, 网络通讯的流程

![img](Initial_Learn.assets/wps2.png)

## 二, 客户端代码案例解析

### 1.创建客户端socket

socket函数用于创建一个套接字

第一个参数指明地址族, 例如AF_INET表示使用ipv4地址

第二个参数表示套接字类型, SOCK_STREAM是用于提供序列化, 可靠双向, 基于字节的链接, 与TCP相关联

第三个参数表示协议, 实参为0时表示采用默认协议, 对于SOCK_STREAM来讲, 默认协议即TCP协议

返回值为套接字描述符(非负整数), 可以用该标识符来指明当前的套接字, 当套接字创建失败时则返回-1

```cpp
// 1.创建客户端socket
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd == -1) {
    std::cerr << "socket error" << std::endl;
    return -1;
}
```

### 2.向服务器发起连接请求

```c
hostent* h = gethostbyname(argv[1]); // 用于存放服务端IP的结构体
```

参数中的`name`表示要查询的主机名(用`IP`地址表示), 返回值为类型`hostent`的结构体指针

- 如果查询成功，函数返回一个指向 `hostent` 结构体的指针，该结构体包含了主机的相关信息。
- 如果查询失败，函数返回 `NULL`

```c
// hostent 声明
struct hostent {
    char  *h_name;           // 主机的官方名称
    char **h_aliases;        // 指向主机别名列表的指针数组
    int    h_addrtype;       // 地址类型，例如AF_INET（IPv4）
    int    h_length;         // 地址长度，例如对于IPv4是4
    char **h_addr_list;      // 指向主机地址的指针数组
};
```

```c
// sockaddr_in的声明
struct sockaddr_in {
    sa_family_t sin_family;  // 地址族，对于IPv4，通常是AF_INET
    in_port_t sin_port;      // 端口号，网络字节序
    struct in_addr sin_addr; // IPv4地址，网络字节序
    unsigned char sin_zero[8]; // 保留，用于对齐
};
```

```cpp
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

- `sockfd`：要连接的套接字的文件描述符。
- `addr`：指向一个 `sockaddr` 结构体的指针，该结构体包含了服务器端套接字的地址信息。
- `addrlen`：`addr` 指向的结构体的大小。

连接成功返回0, 连接失败-1

```cpp
// 2.向服务器发起连接请求
hostent* h = gethostbyname(argv[1]); // 用于存放服务端IP的结构体
if (h == 0) {
    std::cerr << "get host failed" << std::endl;
    close(sockfd);
    return -1;
}
sockaddr_in server_addr; // 用于存放服务端IP和端口的结构体。
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
memcpy(&server_addr.sin_addr, h->h_addr, (size_t)h->h_length); // 指定服务端的IP地址。
server_addr.sin_port = htons((uint16_t)atoi(argv[2]));         // 指定服务端的通信端口。
// 向服务器发起连接请求
if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "connect error" << std::endl;
    close(sockfd);
    return -1;
}
```

### 3.连接成功后与服务端通信, 向服务端发送一个报文后等待回复, 然后发送下一个报文

```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

- `sockfd`：套接字的文件描述符。
- `buf`：指向要发送数据的缓冲区的指针。
- `len`：要发送数据的长度。
- `flags`：通常设置为0，或者可以使用特定的标志来修改发送操作的行为。

如果成功返回发送的字节数，如果出错返回-1



```c
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

- `sockfd`：套接字的文件描述符。
- `buf`：指向接收数据的缓冲区的指针。
- `len`：缓冲区的长度。
- `flags`：通常设置为0，或者可以使用特定的标志来修改接收操作的行为。

如果成功，返回接收的字节数; 

如果连接已关闭，返回0; 

如果出错，返回-1

```c
// 3. 连接成功，与服务端通信，向服务端发送一个报文后等待回复，然后发送下一个报文
char buffer[1024];
constexpr size_t ChatSize = 3; // 通讯次数
for (size_t i = 0; i < ChatSize; ++i) {
    std::snprintf(buffer, sizeof(buffer), "这是第条%zu信息, 编号为%zu:", i, i);
    // 发送请求报文
    if (send(sockfd, buffer, strlen(buffer), 0) <= 0) {
        std::cerr << "send failed!" << std::endl;
        break;
    }
    std::cout << "发送" << buffer << std::endl;
    memset(buffer, 0, sizeof(buffer));

    // 接受服务端的回应报文, 如果没有收到则阻塞等待
    if (recv(sockfd, buffer, sizeof(buffer), 0) <= 0) {
        std::cerr << "recv failed" << std::endl;
        break;
    }
    std::cout << "收到" << buffer << std::endl;
    sleep(1);
}
```

### 4.关闭socket连接, 释放资源
```c
close(sockfd);
```

## 三, 服务端代码案例解析
### 1.创建服务端socket

```c
// 1. 创建服务端的socket
int listenfd = socket(AF_INET, SOCK_STREAM, 0); // 服务端用来监听的socket
if (listenfd == -1) {
    std::cerr << "socket failed!" << std::endl;
    return -1;
}
```
### 2.把服务端用于通信的IP和端口号绑定到服务端socket上

```c
// socket中的bind函数声明
// bind 函数通常在套接字创建后、监听或接受连接前调用。绑定操作允许套接字在特定的端口上监听传入的数据
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
- `sockfd`：要绑定的套接字的文件描述符。
- `addr`：指向一个 `sockaddr` 结构体的指针，该结构体包含了要绑定的网络地址和端口号。
- `addrlen`：`addr` 指向的结构体的大小。

绑定成功返回0, 绑定失败返回-1

```c
// 2. 把服务端用于通信的IP和端口绑定到socket上
sockaddr_in sever_addr;
memset(&sever_addr, 0, sizeof(sever_addr));
sever_addr.sin_family      = AF_INET;                        // 指定协议
sever_addr.sin_addr.s_addr = htonl(INADDR_ANY);              // 服务端任意网卡到IP都可以用于通信
sever_addr.sin_port        = htons((uint16_t)atoi(argv[1])); // 指定端口号
// 绑定服务端的IP和端口
if (bind(listenfd, (sockaddr*)&sever_addr, sizeof(sever_addr)) != 0) {
    std::cerr << "bind failed!" << std::endl;
    close(listenfd);
    return -1;
}
```

### 3.将服务端socket设置为可监听状态

```c
// listen函数声明
// 用于将一个套接字（socket）设置为监听模式，使其能够接收连接请求
// 确保在使用 listen 函数之前已经成功调用了 bind 函数。
int listen(int sockfd, int backlog);
```
- `sockfd`：已经绑定到特定地址的套接字的文件描述符。
- `backlog`：指定内核应该为相应套接字排队的最大挂起连接的数量
 
调用成功返回0, 调用失败返回-1

```c
// 3. 将服务端socket设置为可监听状态
if ((listen(listenfd, 5)) != 0) {
    std::cerr << "listen failed!" << std::endl;
    close(listenfd);
    return -1;
}
```

### 4.受理客户端的连接请求

```c
// accept 函数声明
// accept用于接受客户端的连接请求。
// 当服务器端套接字处于监听模式（由 listen 函数设置）时，accept 函数会从已完成连接的队列中取出第一个连接请求，并为该连接创建一个新的套接字。
// 这个新的套接字用于后续与客户端通信。
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
- `sockfd`：处于监听模式的服务器端套接字的文件描述符。
- `addr`：如果非空，`accept` 函数会填充这个 `sockaddr` 结构体，提供连接客户端的地址信息。
- `addrlen`：指向 `addr` 结构体长度的变量的指针

如果成功，返回一个新的套接字文件描述符，用于与已接受的客户端通信。失败则返回-1

```c
// 4. 受理客户端的连接请求，如果没有连接请求，则accept函数则会阻塞等待
int clientfd = accept(listenfd, 0, 0); // clientfd为客户端连接上来的socket
if (clientfd == -1) {
    std::cerr << "accept failed!" << std::endl;
    close(listenfd);
    return -1;
}
std::cout << "客户端连接请求已经接受" << std::endl;
```

### 5.与客户端通信, 接收客户端发送的信息并发送返回信息

```c
// 5. 与客户端通信，接收客户端发来的信息，返回信息OK
char buffer[1024];
while (true) {
    memset(buffer, 0, sizeof(buffer));
    // 如果客户端没有发送报文, 则陷入阻塞
    // 如果客户端断开连接, 则返回0
    if ((recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) {
        break;
    }
    std::cout << "接收: " << buffer << std::endl;

    // 生成回应报文
    strcpy(buffer, "ok");
    // 向客户端发送回应报文
    if ((send(clientfd, buffer, strlen(buffer), 0)) <= 0) {
        std::cerr << "send failed!" << std::endl;
        break;
    }
    std::cout << "发送: " << buffer << std::endl;
}
```

### 6.关闭socket连接, 释放资源
```c
// 6. 关闭资源
close(listenfd);
close(clientfd);
```

## 四, 运行(案例程序中, IP和端口号由main函数参数指出)
1. 运行服务端，设置监听端口号
```shell
# 端口自行指定, 大于1024且不与已有的端口号冲突就行
./server 5050
```
2. 运行客户端，请求连接对应的服务端的IP和端口号
```shell
# IP地址为服务端所在的IP地址, 端口号为服务端设置的监听端口号
./client 222.195.85.39 5050
```