#!/usr/bin/env python

import math
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

font = {'family' : 'normal',
        'weight' : 'bold',
        'size'   : 40}

import matplotlib

#plt.style.use('jay1')


matplotlib.rc('font', **font)

fig, axs = plt.subplots(2, 2, figsize=(35, 15))

pace_log_filenames = [
]

last_paces = []
for fname in pace_log_filenames:
    last_pace = pd.read_csv(fname)
    last_pace['t'] = last_pace['t'] - last_pace['t'].min()
    last_paces.append(last_pace)

t_vals = []
pedal_vals = []
resist_pct_vals = []
tgt_resist_pct_vals = []
power_vals = []
work_vals = []

log_file = f'logs/log_{time.strftime("%Y-%m-%d_%H%M%S")}.csv'
log_fd = open(log_file, 'w')
print("t,pedal_cnt,resist_pct,tgt_resist_pct,power,work", file=log_fd)

ref_pace = pd.DataFrame({
    "t": np.arange(0, 1200.0, 1.0),
})
ref_pace['pedal_cnt'] = 1000.0 / 1200.0
ref_pace['resist_pct'] = 0.3
ref_pace['power'] = 130.0
ref_pace['work'] = 200.0 / 1200.0

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
    resist_pct = float(parts[1])
    tgt_resist_pct = float(parts[2])
    power = float(parts[3])
    work = float(parts[4])

    t_vals.append(t)
    pedal_vals.append(pedal_cnt)
    resist_pct_vals.append(resist_pct)
    tgt_resist_pct_vals.append(tgt_resist_pct)
    power_vals.append(power)
    work_vals.append(work)

    df = pd.DataFrame({
        "t": t_vals,
        #"hb_cnt": hb_vals,
        "pedal_cnt": pedal_vals,
        "resist_pct": resist_pct_vals,
        #"hb_bpm": hb_bpm_vals,
        "tgt_resist_pct": tgt_resist_pct_vals,
        "power": power_vals,
        "work": work_vals,
    })
    df['t'] = df['t'] - df['t'].min()

    dfs = [
        (ref_pace,  1.0, "blue"),
        (df,        1.0, "red"),
    ]

    ax = axs[0][0]
    ax.clear()
    ax.set_title(f"Resist: {resist_pct}  Target: {tgt_resist_pct}")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.resist_pct, alpha=alph, c=clr)
    ax.plot(df.t, df.tgt_resist_pct, alpha=1.0, c="lightblue")

    ax = axs[0][1]
    ax.clear()
    ax.set_title(f"Pedal Count: {df.pedal_cnt.sum()},  Time: {df.t.max() / 60.0:.1f}m")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.pedal_cnt.cumsum(), alpha=alph, c=clr)

    ax = axs[1][0]
    ax.clear()
    ax.set_title(f"Power: {df.power.values[-1]:.0f} W")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.power, alpha=alph, c=clr)

    ax = axs[1][1]
    ax.clear()
    ax.set_title(f"Work: {df.work.sum():.0f} kcal")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.work.cumsum(), alpha=alph, c=clr)

    print(f"{t},{pedal_cnt},{resist_pct},{tgt_resist_pct},{power},{work}", file=log_fd)
    log_fd.flush()
ani = animation.FuncAnimation(fig, animate, interval=100)
plt.tight_layout()
plt.show()
