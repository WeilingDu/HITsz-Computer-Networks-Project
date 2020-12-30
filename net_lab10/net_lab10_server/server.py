from socket import *
import threading


def server_process(connectionSocket):
    try:
        message = connectionSocket.recv(1024)
        message = message.decode()
        list = message.split()  # 将数据用空格分开
        option = list[0]
        filename = list[1]
        if option == 'download':
            outputdata = '1 '  # 表示有该文件
            f = open(filename, "rb")
            outputdata += f.read().decode()
            f.close()
            connectionSocket.send(outputdata.encode())
            connectionSocket.close()
            print('thread id: ', threading.currentThread())
            print('Download finished!')
        elif option == 'upload':
            data = 1
            while data:
                f = open(filename, 'a+')
                del list[0:2]  # 删掉前两个元素option和filename
                str = ' '
                data = str.join(list)
                print(data)
                f.write(data)
                f.close()
            connectionSocket.close()
            print('Upload finished!')
    except IOError:
        # Send response message for file not found
        outputdata = '0 Not found this file!\r\n\r\n'
        # Close client socket
        for i in range(0, len(outputdata)):
            connectionSocket.send(outputdata[i].encode())
        connectionSocket.close()


if __name__ == '__main__':
    serverSocket = socket(AF_INET, SOCK_STREAM)
    # Prepare a sever socket
    serverHost = '127.0.0.1'
    serverPort = 5000
    serverSocket.bind((serverHost, serverPort))  # 指定ip和端口号
    serverSocket.listen(10)  # 保证挂起的连接池中至少要有10个连接

    while True:
        # Establish the connection
        print('Ready to serve...')
        connectionSocket, addr = serverSocket.accept()
        thread = threading.Thread(target=server_process, args=(connectionSocket,))
        thread.start()

    serverSocket.close()

