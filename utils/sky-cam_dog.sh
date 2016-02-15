#!/bin/bash

sleep 5

t=`ps -A | grep 'run_sky-cam.py' | wc -l`
echo $t
if [ $t -lt 1 ]; then
    $HOME/bin/run_sky-cam.py & #> /dev/null 2> /dev/null &
else
    echo "Sky-Cam is already running!"
fi

