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

gme = 0.2

pace_log_filenames = [
    #"logs/log_2019-02-27_200115.csv",  # jay
    #"logs/log_2019-02-28_200443.csv",  # jay
    #"logs/log_2019-03-01_202539.csv",  # jay
    #"logs/log_2019-03-02_102540.csv",  # jay
    #"logs/log_2019-03-03_195627.csv",  # jay
    #"logs/log_2019-03-04_195042.csv",  # jay
    #"logs/log_2019-03-05_193429.csv",  # jay
    #"logs/log_2019-03-06_202052.csv",  # jay
    "logs/log_2019-03-07_202639.csv",  # jay
    #"logs/log_2019-02-27_202229.csv",  # caleb
    #"logs/log_2019-02-28_202446.csv",  # caleb
]

last_paces = []
for fname in pace_log_filenames:
    last_pace = pd.read_csv(fname)
    last_pace['t'] = last_pace['t'] - last_pace['t'].min()
    last_paces.append(last_pace)

t_vals = []
hb_vals = []
pedal_vals = []
resist_pct_vals = []
hb_bpm_vals = []
tgt_resist_pct_vals = []

log_file = f'logs/log_{time.strftime("%Y-%m-%d_%H%M%S")}.csv'
log_fd = open(log_file, 'w')
print("t,hb_cnt,pedal_cnt,resist_pct,hb_bpm", file=log_fd)

ref_pace = pd.DataFrame({
    "t": np.arange(0, 1200.0, 1.0),
})
ref_pace['pedal_cnt'] = 1000.0 / 1200.0
ref_pace['resist_pct'] = 0.3
ref_pace['hb_bpm'] = 125.0

while True:
    try:
        data, address = sock.recvfrom(4096)
    except socket.error as e:
        time.sleep(0.1)
        continue
    msg = data.decode('utf-8')
    print(msg)

    parts = [v for v in msg.split(',')]

    t = time.time()
    pedal_cnt = int(parts[0])
    hb_cnt = int(parts[1])
    resist_pct = float(parts[2])
    hb_bpm = float(parts[3])
    tgt_resist_pct = float(parts[4])

    t_vals.append(t)
    hb_vals.append(hb_cnt)
    pedal_vals.append(pedal_cnt)
    resist_pct_vals.append(resist_pct)
    hb_bpm_vals.append(hb_bpm)
    tgt_resist_pct_vals.append(tgt_resist_pct)

    df = pd.DataFrame({
        "t": t_vals,
        "hb_cnt": hb_vals,
        "pedal_cnt": pedal_vals,
        "resist_pct": resist_pct_vals,
        "hb_bpm": hb_bpm_vals,
        "tgt_resist_pct": tgt_resist_pct_vals,
    })
    df['t'] = df['t'] - df['t'].min()

    dfs = [
        (ref_pace,  1.0, "blue"),
        (df,        1.0, "red"),
    ]

    for d in last_paces:
        dfs.append((d, 0.2, "black"))

    for d, alph, clr in dfs:
        wndsz = 60
        d['pedal'] = d['pedal_cnt'].cumsum()
        d['pedal_rpm'] = (d['pedal'] - d['pedal'].rolling(wndsz, min_periods=1).min()) * 60.0 / wndsz
        d['pedal_rate'] = d['pedal_rpm'] / 60.0 * math.pi * 2.0
        d['resist_torque_per_vel'] = 1.8 + 9.0 * d['resist_pct'].rolling(wndsz, min_periods=1).mean()
        d['power'] = d['pedal_rate'] * d['pedal_rate'] * d['resist_torque_per_vel']
        d['work'] = (d['power'] * (d['t'] - d['t'].shift(1))).cumsum()


    fig, axs = plt.subplots(3, 2)

    ax = axs[0][0]
    ax.clear()
    ax.set_title(f"Resist: {resist_pct}  Target: {tgt_resist_pct}")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.resist_pct, alpha=alph, c=clr)
    ax.plot(df.t, df.tgt_resist_pct, alpha=1.0, c="lightblue")

    ax = axs[0][1]
    ax.clear()
    ax.set_title(f"Pedal Count: {df.pedal_cnt.sum()}")
    for d, alph, clr in dfs:
        d = d.copy()
        d['pedal_cnt'] = d['pedal_cnt'].cumsum()
        d = d[d.t > df.t.max() - 120.0]
        d = d[d.t < df.t.max()]
        ax.plot(d.t, d.pedal_cnt, alpha=alph, c=clr)

    ax = axs[1][0]
    ax.clear()
    ax.set_title(f"Total Time: {df.t.max() / 60.0:.1f} minutes")
    for d, alph, clr in dfs:
        d = d[d.t > df.t.min()]
        ax.plot(d.t, d.pedal_cnt.cumsum(), alpha=alph, c=clr)

    ax = axs[1][1]
    ax.clear()
    ax.set_title(f"Heart: {hb_bpm} bpm")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.hb_bpm, alpha=alph, c=clr)
    ax.set_ylim(30.0, 200.0)

    ax = axs[2][0]
    ax.clear()
    ax.set_title(f"Power: {df.power.values[-1]:.0f} W")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.power, alpha=alph, c=clr)

    ax = axs[2][1]
    ax.clear()
    ax.set_title(f"Work: {df.work.values[-1] / 4184.0 / gme:.0f} kcal")
    for d, alph, clr in dfs:
        ax.plot(d.t, d.work / 4184.0 / gme, alpha=alph, c=clr)

    #ani = animation.FuncAnimation(fig, animate, interval=100)
    plt.tight_layout()
    #plt.show()
    plt.savefig('live_update.png')

    print(f"{t},{hb_cnt},{pedal_cnt},{resist_pct},{hb_bpm}", file=log_fd)
    log_fd.flush()
