import numpy as np
import pandas as pd
import matplotlib.pylab as plt
import seaborn as sns

# %%

#df_ = pd.read_csv("log_20181122_1_jacob.csv")
#df_ = pd.read_csv("log_20181124_1_jay.csv")
#df_ = pd.read_csv("log_20181124_2_caleb.csv")
df_ = pd.read_csv("log_20181125_1_jay.csv")

# %%
df = df_.copy()
#df['heartF'] = df.heart.ewm(alpha=0.09).mean()
df['t'] = df.index * 0.005
df['heart_ds4'] = df.heart * 0.7 - df.heart.shift(4) + df.heart.shift(7) * 0.3
df['heart_rct_max'] = df.heart_ds4.rolling(400).max()
df['heart_pulse_thresh'] = df['heart_rct_max'] * 0.5
df['heart_pulse'] = np.where(df['heart_ds4'] > df['heart_pulse_thresh'], 1.0, 0.0)
df['heart_pulse_t'] = np.where((df.heart_pulse > 0.5) & (df.heart_pulse.shift(1) < 0.5), df.t, np.nan)
df['heart_pulse_t'] = df.heart_pulse_t.ffill()
df['heart_pulse_t'] = (df['heart_pulse_t'] - df['heart_pulse_t'].shift(1)).replace(0.0, np.nan).ffill()

df['heart_bpm'] = 60.0 / df['heart_pulse_t']
df['heart_bpm_delta'] = df['heart_bpm'] - df['heart_bpm'].shift(400)
df['heart_bpm'] = np.where(np.abs(df['heart_bpm_delta']) < 10, df['heart_bpm'], np.nan)
df['heart_bpm'] = df['heart_bpm'].ffill()
#df = df[4000:5000]

fig, axs = plt.subplots(5, 1, figsize=(15, 10))
plt.sca(axs[0])
plt.plot(df.heart)
plt.grid()
plt.legend()

plt.sca(axs[1])
plt.plot(df.heart_ds4)
plt.plot(df.heart_pulse_thresh)
plt.grid()
plt.legend()

plt.sca(axs[2])
plt.plot(df.heart_pulse)
plt.grid()
plt.legend()

plt.sca(axs[3])
plt.plot(df['heart_bpm'])
plt.ylim(40.0, 200.0)
plt.grid()
plt.legend()

plt.sca(axs[4])
plt.plot(df['speedo'])
plt.grid()
plt.legend()

plt.show()
