import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# %%
logs = [
    "logs/log_2019-02-27_200115.csv",  # jay
    "logs/log_2019-02-28_200443.csv",  # jay
    "logs/log_2019-03-01_202539.csv",  # jay
    "logs/log_2019-03-02_102540.csv",  # jay
    "logs/log_2019-03-03_195627.csv",  # jay
    "logs/log_2019-03-04_195042.csv",  # jay
    #"logs/log_2019-02-27_202229.csv",  # caleb
    #"logs/log_2019-02-28_202446.csv",  # caleb
]

plt.figure(figsize=(12, 5))
for l in logs:
    df = pd.read_csv(l)
    df['t'] = pd.to_datetime(df['t'], unit='s')
    df['t'] = df['t'] - df['t'].min()
    df = df.set_index('t')
    df['pedal'] = df['pedal_cnt'].cumsum()
    df['pedal_rate'] = df['pedal'] - df['pedal'].rolling('60s').min()
    #plt.plot(df.pedal_cnt.cumsum() - df.pedal_cnt.cumsum().shift(60))
    plt.plot(df['pedal_rate'], label=l)
plt.legend()
plt.grid()
plt.show()

plt.figure(figsize=(12, 5))
for l in logs:
    df = pd.read_csv(l)
    df['t'] = pd.to_datetime(df['t'], unit='s')
    df['t'] = df['t'] - df['t'].min()
    df = df.set_index('t')
    df['cnt'] = df['hb_cnt'].cumsum()
    df['rate'] = df['cnt'] - df['cnt'].rolling('60s').min()
    plt.plot(df['rate'], label=l)
plt.legend()
plt.ylim(40, 170)
plt.grid()
plt.show()
