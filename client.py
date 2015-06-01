import socket



s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('58.192.114.239',10086))
#s.connect(('127.0.0.1',10086))
s.send("\x01" + "87c0bb001d10fbf11874589e0ac7823f" + "\x09213121455" + "\x09213121455" + "\n"*100)
print s.recv(50)
s.close()