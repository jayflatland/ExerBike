#!/usr/bin/env python

import time
import pychromecast

import threading
import time

import http.server
import socketserver

def thread_function():
    PORT = 8000
    Handler = http.server.SimpleHTTPRequestHandler
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        # httpd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        print("serving at port", PORT)
        httpd.serve_forever()

x = threading.Thread(target=thread_function)
x.start()

print("Finding chromecasts...")
chromecasts = pychromecast.get_chromecasts()

cast = next(cc for cc in chromecasts if cc.device.friendly_name == "Basement TV 2")

print("Connecting...")
cast.wait()

print("Playing...")
mc = cast.media_controller
#mc.play_media('http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4', 'video/mp4')
mc.play_media('http://10.1.10.11:8000/IMG_20190814_181123.jpg', 'image/jpg')
mc.block_until_active()

# mc.pause()
# time.sleep(5)
#mc.play()

while True:
    time.sleep(10.0)