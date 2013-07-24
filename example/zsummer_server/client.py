import socket


s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#s.bind(localip, localport)
s.connect(("127.0.0.1",81))
s.send("aaaaaabbbbbbbaaa", 10)
s.close()

