cd /Users/liuyifeng/Desktop/code/NET_CPP_Learn/build/NET_SRC/Poll

# 检查是否提供了足够的参数
if [ "$#" -ne 3 ]; then
  echo "Usage: $0 <number_of_times> <IP_address> <port_number>"
  exit 1
fi

# 从命令行参数获取执行次数、IP地址和端口号
x=$1
IP=$2
PORT=$3

# 循环x次执行Client_5命令
for (( i=0; i<x; i++ ))
do
  ./Client_5 $IP $PORT
done