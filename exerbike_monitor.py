#!/usr/bin/env python

import os
import socket
#import sys
from pandas import Timestamp, Timedelta

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('', 10245)
print('Listening on port %s' % server_address[1])
sock.bind(server_address)

while True:
    print('\nwaiting to receive message')
    data, address = sock.recvfrom(4096)
    print(data)
