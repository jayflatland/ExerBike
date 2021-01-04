#!/usr/bin/env python

import random
import os
import time
import pychromecast

import threading
import time

import http.server
import socketserver
import random



def thread_function():
    PORT = 8000
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), http.server.SimpleHTTPRequestHandler) as httpd:
        # httpd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        print("serving at port", PORT)
        httpd.serve_forever()

if 0:
    thread_function()
else:
    x = threading.Thread(target=thread_function)
    x.start()

    print("Finding chromecasts...")
    chromecasts = pychromecast.get_chromecasts()

    cast = next(cc for cc in chromecasts if cc.device.friendly_name == "Basement TV 2")

    print("Connecting...")
    cast.wait()

    print("Playing...")
    mc = cast.media_controller
    
    while True:
        mc.play_media(f'http://10.1.10.11:8000/live_update.png?stupid={random.random()}', 'image/png', stream_type="LIVE")
        mc.block_until_active()
        time.sleep(1.0)