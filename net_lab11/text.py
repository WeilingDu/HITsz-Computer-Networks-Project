from socket import *
import base64
subject = 'I love computer networks!'
contenttype = 'text/plain'
text_msg = '\r\n I love computer networks!'
endmsg = '\r\n.\r\n'

# Choose a mail server (e.g. Google mail server)
# and call it mailserver
mailserver = ('smtp.qq.com', 587)

# Sender and reciever
sender = '506271944@qq.com'
senderpwd = 'dcccqynhzinqbgic'
receiver = 'kairose_em@163.com'

# Auth information (Encode with base64)
# 账户和密码需要经过base64编码加密
user = base64.b64encode(sender.encode()) + b'\r\n'
password = base64.b64encode(senderpwd.encode()) + b'\r\n'

# Create socket called clientSocket
# and establish a TCP connection with mailserver
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect(mailserver)

recv = clientSocket.recv(1024).decode()
print(recv)
if recv[:3] != '220':
    print('220 reply not received from server.')

# Send HELO command and print server response.
heloCommand = 'HELO Alice\r\n'
clientSocket.send(heloCommand.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# Auth
# Send MAIL FROM command and print server response.
login = 'AUTH LOGIN\r\n'
clientSocket.send(login.encode())
recv2 = clientSocket.recv(1024).decode()
print('login: ', recv2)

clientSocket.send(user)
recv2 = clientSocket.recv(1024).decode()
print('user: ', recv2)
clientSocket.send(password)
recv3 = clientSocket.recv(1024).decode()
print('password: ', recv3)

mailFrom = 'MAIL FROM: <506271944@qq.com>\r\n'
clientSocket.send(mailFrom.encode())
recv4 = clientSocket.recv(1024).decode()
print('mail from: ', recv4)

# Send RCPT TO command and print server response.
reptTo = 'RCPT TO: <kairose_em@163.com>\r\n'
clientSocket.send(reptTo.encode())
recv5 = clientSocket.recv(1024).decode()
print('rcpt to: ', recv5)

# Send DATA command and print server response.
data = 'DATA\r\n'
clientSocket.send(data.encode())
recv6 = clientSocket.recv(1024).decode()
print('data: ', recv6)

# Send message data.
msg = 'From: ' + sender + '\r\n'
msg += 'To: ' + receiver + '\r\n'
msg += 'Subject: ' + subject + '\r\n'
msg += 'Content-Type: ' + contenttype + '\r\n'
msg += text_msg
clientSocket.send(msg.encode())

# Message ends with a single period.
clientSocket.send(endmsg.encode())
recv9 = clientSocket.recv(1024).decode()
print('mail: ', recv9)


# Send QUIT command and get server response.
quitCommand = 'QUIT\r\n'
clientSocket.send(quitCommand.encode())
recv10 = clientSocket.recv(1024).decode()
print('quit: ', recv10)


# Close connection
clientSocket.close()


