from socket import *

serverHost = '127.0.0.1'
serverPort = 5000


def download_file(filename):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverHost, serverPort))
    # 发送要下载文件的命令以及文件名给服务器
    outputdata = 'download ' + filename
    clientSocket.send(outputdata.encode())
    data = 1
    while data:
        data = clientSocket.recv(1024)
        data = data.decode()
        # 创建文件，写入内容
        f = open(filename, 'a+')
        f.write(data)
        f.close()
    clientSocket.close()
    print('Download finished!\n')


def upload_file(filename):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverHost, serverPort))
    outputdata = 'upload ' + filename + ' '

    f = open(filename, "rb")
    outputdata += f.read().decode()
    f.close()
    # for i in range(0, len(outputdata)):
    clientSocket.send(outputdata.encode())
    clientSocket.close()
    print('Upload finished!\n')
