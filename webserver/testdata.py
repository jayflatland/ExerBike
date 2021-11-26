import numpy as np
import pandas as pd
from pandas import Timestamp, Timedelta
import matplotlib
import matplotlib.pylab as plt


now = Timestamp('now')

nowstr = str(now)
datestr = nowstr[:10]
df = pd.read_csv(f"/opt/ExerBike/logs/{datestr}_exerbike.csv")

df.set_index(pd.DatetimeIndex(df['ts']), inplace=True)
df['ts'] = df.index
df['dts'] = df['ts'].diff()

df['workout_gap'] = (df['dts'] > Timedelta('30s')) | df['dts'].isnull()

df['workout_start_secs'] = np.where(df['workout_gap'], (df['ts'] - now).dt.total_seconds(), np.nan)
df['workout_start_secs'] = df['workout_start_secs'].ffill()

df['is_latest_work'] = df['workout_start_secs'] == df['workout_start_secs'].max()
df = df[df['is_latest_work']]

# %%

sz = (6, 5)
plt.figure(figsize=sz)
plt.title("Total Calories")
plt.grid()
plt.plot(df.calories.cumsum())
plt.show()

plt.figure(figsize=sz)
plt.title("Resistance")
plt.grid()
plt.plot(df.resistance)
plt.plot(df.targetResist)
plt.show()