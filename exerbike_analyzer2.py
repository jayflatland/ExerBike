import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# %%
l = "logs/log_2019-03-06_202052.csv"

df = pd.read_csv(l)
#df['t'] = pd.to_datetime(df['t'], unit='s')
df['t'] = df['t'] - df['t'].min()
df['pedal'] = df['pedal_cnt'].cumsum()
df['pedal_rpm'] = (df['pedal'] - df['pedal'].rolling(20).min()) / 20.0 * 60.0
df['pedal_rate'] = df['pedal_rpm'] / 60.0 * math.pi * 2.0
df['resist_torque_per_vel'] = 1.8 + 9.0 * df['resist_pct']
df['power'] = df['pedal_rate'] * df['pedal_rate'] * df['resist_torque_per_vel']
df['work'] = (df['power'] * (df['t'] - df['t'].shift(1))).cumsum()
df = df.set_index('t')


plt.figure(figsize=(12, 5))
plt.plot(df['power'])
plt.legend()
plt.grid()
plt.show()

plt.figure(figsize=(12, 5))
plt.plot(df['work'] / 4184 / .2)
plt.legend()
plt.grid()
plt.show()
