import socket

HOST = 'localhost'
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    password = input("Enter a password to check: ")
    s.sendall(password.encode())
    data = s.recv(1024)
print(f"Server response:\n{data.decode()}")