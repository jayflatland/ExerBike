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


cur_logfile_date = None
logfd = None

while True:
    # print('\nwaiting to receive message')
    data, address = sock.recvfrom(4096)
    #print(data)

    try:
        msg = data.decode('utf-8')
        parts = msg.split(',')
        if len(parts) != 4:
            continue
        resistance, targetResist, calories = [float(e) for e in parts[1:]]
    except Exception as e:
        print(e)
        continue

    nowstr = str(Timestamp('now'))
    datestr = nowstr[:10]
    
    if cur_logfile_date != datestr:
        if cur_logfile_date is not None:
            print(f"Closing log {cur_logfile_date}")
            logfd.close()
        cur_logfile_date = datestr
        logfilepath = f"logs/{datestr}_exerbike.csv"
        if os.path.exists(logfilepath):
            print(f"Appending to log {logfilepath}...")
            logfd = open(logfilepath, "a")
        else:
            print(f"Creating log {logfilepath}...")
            logfd = open(logfilepath, "w")
            print("ts,resistance,targetResist,calories", file=logfd)

    print(f"{nowstr},{resistance},{targetResist},{calories}", file=logfd)
    # print(f"{nowstr},{resistance},{targetResist},{calories}")
    logfd.flush()
