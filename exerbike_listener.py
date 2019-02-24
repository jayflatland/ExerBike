import socket
import sys

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('', 10245)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)
sock.setblocking(0)

fig, axs = plt.subplots(2, 1)

hb_vals = []
pedal_vals = []
hb_cnt_cumsum = 0
pedal_cnt_cumsum = 0

log_file = f'logs/log_{time.strftime("%Y-%m-%d_%H%M%S")}.csv'
log_fd = open(log_file, 'w')
print("t,hb,pedal,resistance", file=log_fd)
def animate(i):
    try:
        data, address = sock.recvfrom(4096)
    except socket.error as e:
        return
    msg = data.decode('utf-8')

    parts = [v for v in msg.split(',')]
    pedal_cnt = int(parts[0])
    hb_cnt = int(parts[1])
    resist_pct = float(parts[2])

    global hb_cnt_cumsum, pedal_cnt_cumsum
    hb_cnt_cumsum += hb_cnt
    pedal_cnt_cumsum += pedal_cnt
    hb_vals.append(hb_cnt_cumsum)
    pedal_vals.append(pedal_cnt_cumsum)

    axs[0].clear()
    axs[0].plot(hb_vals)

    axs[1].clear()
    axs[1].plot(pedal_vals)
    print(f"{time.time()},{hb_cnt},{pedal_cnt},{resist_pct}", file=log_fd)
    log_fd.flush()
    print(f"{hb_cnt_cumsum},{pedal_cnt_cumsum},{resist_pct}")
ani = animation.FuncAnimation(fig, animate, interval=100)
plt.show()
