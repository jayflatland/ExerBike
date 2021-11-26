#!/bin/sh

if [ "$1" = "debug" ]; then
    echo "DEBUG MODE!"
    export FLASK_APP=webserver.py
    export FLASK_ENV=development
    export FLASK_DEBUG=1
    flask run --host=0.0.0.0 --port=32010
elif [ "$1" = "test" ]; then
    echo "TEST MODE!"
    gunicorn server:app -w 4 -b 0.0.0.0:32011 --timeout 60
else
    echo "PROD MODE!"
    echo "Setting ulimit to 1GB of RAM..."
    ulimit -v 1048576
    gunicorn webserver:app -w 16 -b 0.0.0.0:32010 --timeout 60 --access-logfile logs/access.log
fi
