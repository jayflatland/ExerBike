import math
import numpy as np
import pandas as pd
import matplotlib.pylab as plt
import seaborn as sns

# %%


rotational_rate = 5.2  # rad/s
resistance = 7.0  # Nms/rad

dt = 0.01
t = 0.0
work = 0.0

rows = []
for i in range(7000):
    power = rotational_rate * rotational_rate * resistance
    work += power * dt
    t += dt

    rows.append({
        "t": t,
        "power": power,
        "work": work,
    })

df = pd.DataFrame(rows)
figsize=(12, 3)
plt.figure(figsize=figsize)
plt.title("Power")
plt.plot(df.t, df.power)
plt.legend()
plt.grid()
plt.show()

plt.figure(figsize=figsize)
plt.title("Work (kcal)")
plt.plot(df.t, df.work / 4184.0)
plt.legend()
plt.grid()
plt.show()
