import socket
import sys

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

# %%

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('', 10245)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)
sock.setblocking(0)

fig, axs = plt.subplots(2, 2)

df_pace = pd.read_csv("logs/log_2019-02-25_195635.csv")
df_pace['t'] = df_pace['t'] - df_pace['t'].min()

t_vals = []
hb_vals = []
pedal_vals = []
resist_pct_vals = []
hb_bpm_vals = []

log_file = f'logs/log_{time.strftime("%Y-%m-%d_%H%M%S")}.csv'
log_fd = open(log_file, 'w')
print("t,hb,pedal,resistance,hb_bpm", file=log_fd)

pace = pd.DataFrame({
    "t": np.arange(0, 1200.0, 1.0),
})
pace['pedal'] = 1000.0 / 1200.0
pace['resist_pct'] = 0.3

def animate(i):
    try:
        data, address = sock.recvfrom(4096)
    except socket.error as e:
        return
    msg = data.decode('utf-8')
    #print(msg)

    parts = [v for v in msg.split(',')]

    t = time.time()
    pedal_cnt = int(parts[0])
    hb_cnt = int(parts[1])
    resist_pct = float(parts[2])
    hb_bpm = float(parts[3])

    t_vals.append(t)
    hb_vals.append(hb_cnt)
    pedal_vals.append(pedal_cnt)
    resist_pct_vals.append(resist_pct)
    hb_bpm_vals.append(hb_bpm)

    df = pd.DataFrame({
        "t": t_vals,
        "hb": hb_vals,
        "pedal": pedal_vals,
        "resist_pct": resist_pct_vals,
        "hb_bpm": hb_bpm_vals,
    })
    df['t'] = df['t'] - df['t'].min()

    axs[0][0].clear()
    axs[0][0].set_title(f"Resist: {resist_pct}")
    axs[0][0].plot(df.t, df.resist_pct)
    axs[0][0].plot(pace.t, pace.resist_pct)
    axs[0][0].plot(df_pace.t, df_pace.resistance)

    axs[0][1].clear()
    axs[0][1].set_title(f"Pedal Count: {df.pedal.sum()}")
    axs[0][1].plot(df.t, df.pedal.cumsum())
    pace2 = pace[(pace.t > df.t.min()) & (pace.t < df.t.max())]
    axs[0][1].plot(pace2.t, pace2.pedal.cumsum())
    pace2 = df_pace[(df_pace.t > df.t.min()) & (df_pace.t < df.t.max())]
    axs[0][1].plot(pace2.t, pace2.pedal.cumsum())

    axs[1][0].clear()
    axs[1][0].set_title(f"Total Time: {df.t.max() / 60.0:.1f} minutes")
    axs[1][0].plot(df.t, df.pedal.cumsum())
    axs[1][0].plot(pace.t, pace.pedal.cumsum())
    axs[1][0].plot(df_pace.t, df_pace.pedal.cumsum())

    axs[1][1].clear()
    axs[1][1].set_title(f"Heart: {hb_bpm} bpm")
    axs[1][1].plot(df.t, df.hb_bpm)
    axs[1][1].set_ylim(30.0, 200.0)

    print(f"{t},{hb_cnt},{pedal_cnt},{resist_pct},{hb_bpm}", file=log_fd)
    log_fd.flush()
ani = animation.FuncAnimation(fig, animate, interval=100)
plt.show()
