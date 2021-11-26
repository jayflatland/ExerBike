import flask
from flask import request
# import flask_autoindex

import numpy as np
import pandas as pd
from pandas import Timestamp, Timedelta
import matplotlib
matplotlib.use('agg')
import matplotlib.pylab as plt

app = flask.Flask("server")

def img_from_plt(plt):
    import base64
    class StringFile:
        def __init__(self):
            self.totalbuf = b""
        def write(self, buf):
            self.totalbuf += buf
        def seek(self, pos):
            pass  # trick to look like file handle
    string_fd = StringFile()
    plt.savefig(string_fd, edgecolor=plt.gcf().get_edgecolor(), facecolor=plt.gcf().get_facecolor(), format='png')
    return f'<img src="data:image/png;base64,{base64.b64encode(string_fd.totalbuf).decode("utf-8")}">'



@app.route('/exerbike')
def handle_exerbike():
    import time

    now = Timestamp('now')

    nowstr = str(now)
    datestr = nowstr[:10]

    df = pd.read_csv(f"/opt/ExerBike/logs/{datestr}_exerbike.csv")

    df.set_index(pd.DatetimeIndex(df['ts']), inplace=True)
    df['ts'] = df.index


    # trim to the latest workout (anything)
    df['dts'] = df['ts'].diff()

    df['workout_gap'] = (df['dts'] > Timedelta('30s')) | df['dts'].isnull()

    df['workout_start_secs'] = np.where(df['workout_gap'], (df['ts'] - now).dt.total_seconds(), np.nan)
    df['workout_start_secs'] = df['workout_start_secs'].ffill()

    df['is_latest_work'] = df['workout_start_secs'] == df['workout_start_secs'].max()
    df = df[df['is_latest_work']]

    calories_graph = ""
    resistance_graph = ""
    if len(df) > 1:
        # lprint("Plotting...")
        sz = (6, 5)
        plt.figure(figsize=sz)
        plt.title("Total Calories")
        plt.grid()
        plt.plot(df.calories.cumsum())
        calories_graph = img_from_plt(plt)
        plt.close()

        plt.figure(figsize=sz)
        plt.title("Resistance")
        plt.grid()
        plt.plot(df.resistance)
        plt.plot(df.targetResist)
        resistance_graph = img_from_plt(plt)
        plt.close()

    html = f"""
    <html>
    <head>
    <title>ExerBike</title>
    <meta http-equiv="refresh" content="2">
    <script src="https://code.jquery.com/jquery-3.3.1.js"></script>
    <script src="https://cdn.datatables.net/1.10.19/js/jquery.dataTables.min.js"></script>
    <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/1.10.19/css/jquery.dataTables.min.css">
    <style>
    body {{
        font-family: verdana;
    }}
    table.downtime {{
        border-collapse: collapse;
        border-color: #888888;
    }}
    table.downtime td, table.downtime th {{
        padding: 2px 10px 2px 10px;
    }}
    table.downtime th {{
        background-color: #CCCCCC;
    }}
    .agebox {{
        display: inline-block;
        padding: 2px 10px 2px 10px;
        border: 1px solid black;
    }}
    .agebox_title {{
        font-style: bold;
        color: #222222;
    }}

    .agebox_last_data {{
        font-style: italic;
        font-size: 8pt;
        color: #aaaaaa;
    }}
    </style>
    </head>
    <body>
    <span>{calories_graph}{resistance_graph}</span>
    </body>
    </html>
    """

    return html


