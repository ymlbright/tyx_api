import socket

s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1',10086))

s.send("\x01" + "1"*32 + "\x09213121455" + "\x09213121455" + "\n"*100)

print s.recv(1024)