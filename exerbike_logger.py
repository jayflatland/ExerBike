#!/usr/bin/env python

import math
import socket
import sys

# import numpy as np
# import pandas as pd
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
import time

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('', 10245)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)
#sock.setblocking(0)

cur_logfile_date = None
logfd = None


while True:
    try:
        data, address = sock.recvfrom(4096)
    except socket.error as e:
        continue
    msg = data.decode('utf-8').strip()
    parts = msg.split(',')
    if len(parts) != 4:
        continue
    try:
        resistance, targetResist, calories = [float(e) for e in parts[1:]]
    except:
        continue
    print(resistance, targetResist, calories)





import socket
#import sys
from pandas import Timestamp, Timedelta

# Sump is on channel 4
#class Logger:
    


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('', 10245)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)


cur_logfile_date = None
logfd = None

while True:
    # print('\nwaiting to receive message')
    data, address = sock.recvfrom(4096)
    #print(data)
    msg = data.decode('utf-8')

    parts = [float(v) for v in msg.split(',')]
    if len(parts) != 4:
        continue
    try:
        resistance, targetResist, calories = [float(e) for e in parts[1:]]
    except:
        continue

    nowstr = str(Timestamp('now'))
    datestr = nowstr[:10]
    #datestr = nowstr[:18]
    
    if cur_logfile_date != datestr:
        if cur_logfile_date is not None:
            print(f"Closing log {cur_logfile_date}")
            logfd.close()
        cur_logfile_date = datestr
        print(f"Opening log {cur_logfile_date}")
        logfd = open(f"logs/{datestr}_housepower.csv", "w")
        print("ts,resistance,targetResist,calories", file=logfd)

    print(f"{nowstr},{amps_1},{amps_2},{amps_3},{amps_4}", file=logfd)
    logfd.flush()
    #db.query("INSERT INTO power_hawk ( amps_1, amps_2, amps_3, amps_4 ) VALUES ( {}, {}, {}, {} );".format(amps_1, amps_2, amps_3, amps_4))
