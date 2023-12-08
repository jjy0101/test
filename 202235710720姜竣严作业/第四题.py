import socket
import time
import board
import adafruit_dht

# 定义DHT传感器的类型和引脚
dht_sensor = adafruit_dht.DHT11(board.D4)

# 定义服务器的IP地址和端口号
SERVER_IP = '0.0.0.0'
SERVER_PORT = 8888

# 创建TCP服务器套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((SERVER_IP, SERVER_PORT))
server_socket.listen(1)  # 同时最多处理1个连接请求

print('TCP服务器已启动，等待连接...')

while True:
    # 等待客户端连接
    client_socket, client_address = server_socket.accept()
    print('收到来自 %s:%s 的连接请求' % client_address)

    while True:
        # 读取温湿度传感器数据
        try:
            temperature = dht_sensor.temperature
            humidity = dht_sensor.humidity
            data = 'Temperature: %0.1f C\tHumidity: %0.1f %%' % (temperature, humidity)
            print('读取到传感器数据：%s' % data)
        except Exception as e:
            print('读取传感器数据出错：', e)
            data = 'Error'

        # 发送数据到客户端
        try:
            client_socket.sendall(data.encode())
            print('已发送数据：', data)
        except Exception as e:
            print('发送数据出错：', e)
            break

        time.sleep(1)  # 每秒读取并发送一次数据

    # 关闭客户端连接
    client_socket.close()
    print('客户端连接断开')

# 关闭服务器套接字
server_socket.close()
print('TCP服务器已关闭')