import math
import numpy as np
import pandas as pd
import matplotlib.pylab as plt
import seaborn as sns

# %%

"""
Model
10lb force on one pedal
Pedal lever is about 9 inches long

Rotates pedals from flat to down (90 degrees)
0% resist - 1.5s (estimated)
100% resist - 6s (estimated)
Rotational mass is about ??? (units)
Assume resistance force is proportional to speed

The torque applied by the weight dissipates as the weight reaches bottom
The resistance torque is proportional to and in opposition of speed

"""


# pounds2N = 4.44822
# ft2m = 0.3048
# lbf2Nm = pounds2N * ft2m
Nm2lbf = 0.737562

weight = 26.6  # N, 6.0 lb
lever_length = 0.22  # m, 9 inches

# 15 at 100% (takes about 6s)
# 15 at 100% (takes about 6s)
resist_ratio = 1.0
resist_torque_per_vel = 1.8 + 9.0 * resist_ratio
angular_mass = 1.0

# initial conditions
ang_vel = 0.0
a = 0.0
dt = 0.001
t = 0.0
torque = 0.0

rows = []
for i in range(7000):
    w_torque = math.cos(a) * weight * lever_length
    r_torque = -ang_vel * resist_torque_per_vel
    power = abs(r_torque * ang_vel) * lever_length
    torque = w_torque + r_torque
    ang_vel += torque / angular_mass * dt
    a += ang_vel * dt
    t += dt

    rows.append({
        "t": t,
        "a": a,
        "w_torque": w_torque,
        "r_torque": r_torque,
        "torque": torque,
        "power": power,
        "ang_vel": ang_vel,
    })

df = pd.DataFrame(rows)
figsize=(12, 3)
plt.figure(figsize=figsize)
plt.title("Torque (Nm)")
plt.plot(df.t, df.w_torque * Nm2lbf)
plt.plot(df.t, df.r_torque * Nm2lbf)
plt.plot(df.t, df.torque * Nm2lbf)
plt.legend()
plt.grid()
plt.show()

plt.figure(figsize=figsize)
plt.title("Ang Vel (deg/s)")
plt.plot(df.t, df.ang_vel * 180.0 / math.pi)
plt.grid()
plt.show()

plt.figure(figsize=figsize)
plt.title("Power (W)")
plt.plot(df.t, df.power)
plt.grid()
plt.show()

plt.figure(figsize=figsize)
plt.title("Angle (deg)")
plt.plot(df.t, df.a * 180.0 / math.pi)
plt.grid()
plt.show()

