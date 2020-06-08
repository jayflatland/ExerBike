import numpy as np
import pandas as pd
import matplotlib.pylab as plt
import seaborn as sns

from statsmodels.discrete.discrete_model import Logit
from sklearn.neural_network import MLPClassifier
from sklearn.linear_model import LogisticRegression

# %%
features = []
for i in range(500):
    c = f'l_{i}'
    features.append(c)

def load_data(filename):
    df = pd.read_csv(filename)
    for i in range(500):
        c = f'l_{i}'
        df[c] = df['heart'].shift(i)
    df = df.dropna()
    return df

df_trn = load_data("log_20200607_1__jay__labeled.csv")
df_tst = load_data("log_20181125_1_jay.csv")

# %%
    

#m = Logit(df['heart_label'], df[features]).fit()
#f = MLPClassifier()
#f = LogisticRegression()
#m = f.fit(df_trn[features], df_trn['heartbeat_label'])

# df_tst['heartbeat_est'] = f.predict(df_tst[features])

# %%

import torch
import torch.nn as nn
import torch.nn.functional as F

class Net(nn.Module):
    def __init__(self):
      super(Net, self).__init__()
      self.conv1 = nn.Conv2d(1, 32, 3, 1)
      self.conv2 = nn.Conv2d(32, 64, 3, 1)
      self.dropout1 = nn.Dropout2d(0.25)
      self.dropout2 = nn.Dropout2d(0.5)
      self.fc1 = nn.Linear(9216, 128)
      self.fc2 = nn.Linear(128, 10)

    # x represents our data
    def forward(self, x):
      # Pass data through conv1
      x = self.conv1(x)
      # Use the rectified-linear activation function over x
      x = F.relu(x)

      x = self.conv2(x)
      x = F.relu(x)

      # Run max pooling over x
      x = F.max_pool2d(x, 2)
      # Pass data through dropout1
      x = self.dropout1(x)
      # Flatten x with start_dim=1
      x = torch.flatten(x, 1)
      # Pass data through fc1
      x = self.fc1(x)
      x = F.relu(x)
      x = self.dropout2(x)
      x = self.fc2(x)

      # Apply softmax to x
      output = F.log_softmax(x, dim=1)
      return output

random_data = torch.rand((1, 1, 28, 28))

my_nn = Net()
print(my_nn)
result = my_nn(random_data)
print (result)


# %%
if 0:
    r, c = 1, 1
    fig, axs = plt.subplots(r, c, figsize=(15, 10), sharex=True)
    if r == 1 and c == 1: axs = [axs]
    elif r == 1 or c == 1:  axs = list(axs)
    else: axs = [axs[i, j] for j in range(c) for i in range(r)]  # flatten
    
    df = df_tst
    d = df[df['heartbeat_est']]
    plt.sca(axs.pop(0))
    plt.scatter(d.index, d['heart'], c='red')
    #plt.scatter(d2.index, d2['heart'], c='green')
    plt.plot(df['heart'])
    plt.grid()
    plt.legend(loc=2)
    
    plt.show()
    
