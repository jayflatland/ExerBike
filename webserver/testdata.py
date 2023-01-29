import numpy as np
import pandas as pd
from pandas import Timestamp, Timedelta
import matplotlib
import matplotlib.pylab as plt


# %%
now = Timestamp('now') - Timedelta('24h')

nowstr = str(now)
datestr = nowstr[:10]
df = pd.read_csv(f"/opt/ExerBike/logs/{datestr}_exerbike.csv")

df.set_index(pd.DatetimeIndex(df['ts']), inplace=True)
df['ts'] = df.index
df['dts'] = df['ts'].diff()

kcal_to_joules = 4184.0
human_mechanical_efficiency = 0.25
df['joules'] = df['calories'] * kcal_to_joules * human_mechanical_efficiency
df['watts'] = df['joules'] / df['dts'].dt.total_seconds()

df['workout_gap'] = (df['dts'] > Timedelta('5m')) | df['dts'].isnull()

df['workout_start_secs'] = np.where(df['workout_gap'], (df['ts'] - now).dt.total_seconds(), np.nan)
df['workout_start_secs'] = df['workout_start_secs'].ffill()

df['is_latest_work'] = df['workout_start_secs'] == df['workout_start_secs'].max()
df = df[df['is_latest_work']]

# %%

r, c = 3, 1
fig, axs = plt.subplots(r, c, figsize=(9, 18), sharex=True)
if r == 1 and c == 1: axs = [axs]
elif r == 1 or c == 1:  axs = list(axs)
else: axs = [axs[i, j] for j in range(c) for i in range(r)]  # flatten

plt.sca(axs.pop(0))
plt.title("Total Calories")
plt.grid()
plt.plot(df.calories.cumsum())

plt.sca(axs.pop(0))
plt.title("Watts")
plt.grid()
plt.plot(df.watts)
#plt.ylim(0, 10e3)

plt.sca(axs.pop(0))
plt.title("Resistance")
plt.grid()
plt.plot(df.resistance)
plt.plot(df.targetResist)

plt.show()