import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# %%
pace_log_filenames = [
    "logs/log_2019-02-27_200115.csv",  # jay
    "logs/log_2019-02-28_200443.csv",  # jay
    "logs/log_2019-03-01_202539.csv",  # jay
    "logs/log_2019-03-02_102540.csv",  # jay
    "logs/log_2019-03-03_195627.csv",  # jay
    "logs/log_2019-03-04_195042.csv",  # jay
    "logs/log_2019-03-05_193429.csv",  # jay
    "logs/log_2019-03-06_202052.csv",  # jay
    "logs/log_2019-03-07_202639.csv",  # jay
]

gme = 0.2
plt.figure(figsize=(15, 10))
for l in pace_log_filenames:
    d = pd.read_csv(l)
    #d['t'] = pd.to_datetime(d['t'], unit='s')
    d['t'] = d['t'] - d['t'].min()

    wndsz = 60
    d['pedal'] = d['pedal_cnt'].cumsum()
    d['pedal_rpm'] = (d['pedal'] - d['pedal'].rolling(wndsz, min_periods=1).min()) * 60.0 / wndsz
    d['pedal_rate'] = d['pedal_rpm'] / 60.0 * math.pi * 2.0
    d['resist_torque_per_vel'] = 1.8 + 9.0 * d['resist_pct'].rolling(wndsz, min_periods=1).mean()
    d['power'] = d['pedal_rate'] * d['pedal_rate'] * d['resist_torque_per_vel']
    d['work'] = (d['power'] * (d['t'] - d['t'].shift(1))).cumsum()
    d = d.set_index('t')
    # plt.plot(d['power'] / gme, label=l)
    plt.plot(d['work'] / 4184 / gme, label=l)

plt.legend()
plt.grid()
plt.show()
