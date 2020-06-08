import numpy as np
import pandas as pd
import matplotlib.pylab as plt
import seaborn as sns

# %%

#df_ = pd.read_csv("log_20181122_1_jacob.csv")
#df_ = pd.read_csv("log_20181124_1_jay.csv")
#df_ = pd.read_csv("log_20181124_2_caleb.csv")
#df_ = pd.read_csv("log_20181125_1_jay.csv")
df_ = pd.read_csv("log_20200607_1__jay.csv")

# %%
df = df_.copy()
#df['heartF'] = df['heart'].ewm(alpha=0.09).mean()
df['t'] = df.index * 0.005
df['heart_ds4'] = df['heart'] * 0.7 - df['heart'].shift(4) + df['heart'].shift(7) * 0.3
df['heart_rct_max'] = df['heart_ds4'].rolling(400).max()
df['heart_pulse_thresh'] = df['heart_rct_max'] * 0.5
df['heart_pulse_calc'] = np.where(df['heart_ds4'] > df['heart_pulse_thresh'], 1.0, 0.0)
df['heart_pulse_t'] = np.where((df['heart_pulse_calc'] > 0.5) & (df['heart_pulse_calc'].shift(1) < 0.5), df.t, np.nan)
df['heart_pulse_t'] = df['heart_pulse_t'].ffill()
df['heart_pulse_t'] = (df['heart_pulse_t'] - df['heart_pulse_t'].shift(1)).replace(0.0, np.nan).ffill()

df['heart_bpm'] = 60.0 / df['heart_pulse_t']
df['heart_bpm_delta'] = df['heart_bpm'] - df['heart_bpm'].shift(400)
df['heart_bpm'] = np.where(np.abs(df['heart_bpm_delta']) < 10, df['heart_bpm'], np.nan)
df['heart_bpm'] = df['heart_bpm'].ffill()
#df = df[4000:5000]

# df['speedo_pulse_t'] = np.where((df['speedo'] > 0.5) & (df['speedo'].shift(1) < 0.5), df.t, np.nan)
# #df['speedo_pulse_t'] = np.where((df['speedo'] > 0.5) & (df['speedo'].shift(1) < 0.5), 1.0, 0.0)
# df['speedo_pulse_t'] = df['speedo_pulse_t'].ffill()
# df['speedo_pulse_t'] = (df['speedo_pulse_t'] - df['speedo_pulse_t'].shift(1)).replace(0.0, np.nan).ffill()
# df['speedo_rpm'] = 60.0 / df['speedo_pulse_t']
# df['speedo_rpm'] = df['speedo_rpm'].fillna(0.0)

r, c = 4, 1
fig, axs = plt.subplots(r, c, figsize=(15, 10), sharex=True)
if r == 1 and c == 1: axs = [axs]
elif r == 1 or c == 1:  axs = list(axs)
else: axs = [axs[i, j] for j in range(c) for i in range(r)]  # flatten

plt.sca(axs.pop(0))
plt.plot(df['heart'])
plt.grid()
plt.legend(loc=2)

plt.sca(axs.pop(0))
#plt.plot(df['heart'])
plt.plot(df['heart_ds4'])
plt.plot(df['heart_pulse_thresh'])
plt.grid()
plt.legend(loc=2)

plt.sca(axs.pop(0))
plt.plot(df['heart_pulse'] / 10)
plt.plot(df['heart_pulse_calc'] - 2)
plt.grid()
plt.legend(loc=2)

plt.sca(axs.pop(0))
plt.plot(df['heart_bpm'])
#plt.ylim(40.0, 200.0)
plt.grid()
plt.legend(loc=2)

# plt.sca(axs[3])
# plt.plot(df['speedo'])
# plt.plot(df['speedo_pulse_t'])
# plt.plot(df['speedo_rpm'])
# plt.grid()
# plt.legend(loc=2)

plt.show()

# %%
# fig, axs = plt.subplots(1, 1, figsize=(15, 10))
# plt.sca(axs)
# plt.plot(df['heart_bpm'])
# plt.ylim(40.0, 200.0)
# plt.grid()
# plt.legend()
# plt.show()
