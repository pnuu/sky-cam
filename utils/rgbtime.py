#!/usr/bin/python

import Image as im
import sys
import os
import numpy as np
import datetime as dt

def rgbtime(fname, x_idx, y_idx):
    '''Calculate time for a given pixel coordinate from sky-cam
    pixel_times image.'''
    k = im.open(fname)
    x,y = k.size

    r,g,b = k.split()
    r = np.array(r).astype('uint32')
    g = np.array(g).astype('uint32')
    b = np.array(b).astype('uint32')

    start_time = 2**16 * (r[0][0] * 2**16 + g[0][0] * 2**8 + b[0][0]) + \
        (r[0][1] * 2**16 + g[0][1] * 2**8 + b[0][1]) + \
        (r[0][2] * 2**16 + g[0][2] * 2**8 + b[0][2])/1000.

    start_time = dt.datetime.utcfromtimestamp(start_time)

    microsecs = 1000 * (r * 2**16 + g * 2**8 + b)

    time_diff = dt.timedelta(microseconds=int(microsecs[y_idx][x_idx]))
    print 'Time from the beginning of the stack:', time_diff
    t = start_time + time_diff

    time_string = t.strftime('%Y-%m-%d %H.%M.%S,') + '%03d UTC' % (t.microsecond/1000)
    
    print time_string

    return t

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print '\nUsage:\n'
        print '\t%s <pixel_times image> <x> <y>\n' % sys.argv[0]
        print 'for example:\n'
        print '\trgbtime.py 2013-01-18_200019.063_UTC_pixel_times.png 343 123'
        print '\nPixel indices start from top left corner, which is [0, 0]'
        print ''
    else:
        fname = sys.argv[1]
        x = int(sys.argv[2])
        y = int(sys.argv[3])

        out = rgbtime(fname, x, y)
