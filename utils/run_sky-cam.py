#!/usr/bin/python

import ephem
import datetime as dt
import os
from math import pi
from time import sleep

# Sun elevation where imaging mode from day-light to night time is changed
sun_limit = -6.0
# Coordinates of the camera
lat = 60.213
lon = 24.823
# Elevation above sea-level (not that important to be exact)
elevation = 20

# Base of the directory structure where the images will be saved
out_dir = '' # adjust!
# Prefix to be added before the default filename format of the stacks
prefix = ''
# Add full path to sky-cam binary if the executable is not in your $PATH
sky_cam_binary = 'sky-cam'
# File to save status message, add absolute path if necessary, otherwise 
# will be saved to out_dir/
run_until_file = 'run_sky-cam_until.txt'
# Video device used for imaging
device = '/dev/video0'
# Filename for 'latest.png' if it is to be saved. Add absolute path if 
# necessary
latest = 'latest.png'

# Length of the stacks
stack_length = 60
latest_length = 60
# Parameters used for running the sky-cam during the day
day_params = '-c %s -e -O %s -L %d' % (device, latest, latest_length)
# Parameters used for running the sky-cam during the night
night_params = '-c %s -M -x -e -O %s -L %d -p %s -s %d' % (device, latest, latest_length, prefix, stack_length)

# NOTHING BELOW THIS SHOULD NOT BE EDITED, UNLESS YOU KNOW WHAT YOU ARE 
# DOING

bin = 'cd ' + out_dir '; ' + sky_cam_binary

prev = None
NIGHT = 0
DAY = 1

while 1:

    sleep(5)

    place = ephem.Observer()
    place.lat = '%f' % lat
    place.lon = '%f' % lon
    place.horizon = '%f' % sun_limit
    place.elevation = elevation
    t_now = place.date.datetime()

    sun = ephem.Sun()
    sun.compute(place)

    # Check which mode should we run, daylight or night
    if sun.alt*180/pi >= sun_limit or prev == NIGHT: # it is day
        # Calculate time from this moment to darkness
        next_setting = place.next_setting(sun).datetime()
        run_length = next_setting - t_now
	fid = open(run_until_file, 'w')
        fid.write('Sky-Cam is running in day-light mode until %s UTC.\n' % place.next_setting(sun))
	fid.close()
        prev = DAY
        # Run command using day-light parameters
        print 'Start day-light mode for %d seconds' % run_length.seconds
        os.system(bin+' '+day_params+' -l %d' % run_length.seconds)
    else: # it is night
        # Calculate time from this moment to morning twilight
        next_rising = place.next_rising(sun).datetime()
        run_length = next_rising - t_now
	fid = open(run_until_file, 'w')
        fid.write('Sky-Cam is running in night mode until %s UTC.\n' % place.next_rising(sun))
	fid.close()
        prev = NIGHT
        print 'Start night-time mode for %d seconds' % run_length.seconds
        # Run command using night-time parameters
        print bin+' '+night_params+' -l %d' % run_length.seconds
        os.system(bin+' '+night_params+' -l %d' % run_length.seconds)

        
