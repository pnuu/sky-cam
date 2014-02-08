#!/usr/bin/python

from PIL import Image as im
from PIL import ImageDraw as imd
from PIL import ImageFont as imf
import numpy as np
import sys
from glob import glob
import datetime as dt

def get_start_time(fname):
    '''Calculate the stack starting time'''
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

    return start_time


def img_time(fname):
    '''Calculate time for each image pixel.'''
    k = im.open(fname)
    x,y = k.size

    r,g,b = k.split()
    r = np.array(r).astype('uint32')
    g = np.array(g).astype('uint32')
    b = np.array(b).astype('uint32')

    millisecs = r * 2**16 + g * 2**8 + b

    return millisecs


def sc2frames(img_fname, pix_times_fname, 
              fps=25, frame_prefix='anim_', 
              t1=0, t2=None, bg=None, annotate=False):
    '''Create frames from peak-hold image and corresponding
    pixel_times image.

    Required arguments:
        - img_fname -- peak-hold image
        - pix_times_fname -- pixel time image
    Optional arguments:
        - fps -- output framerate (set close to camera frame rate)
        - frame_prefix -- prefix for output files
        - t1 -- start time in ms relative to stack start
        - t2 -- end time in ms relative to stack start
        - bg -- additional image filename for background
        - annotate -- True if timestamps should be added to images
    '''
    img = np.array(im.open(img_fname))
    times = img_time(pix_times_fname)

    dif = np.abs(times - t1)
    closest = np.min(dif)
    idx = np.where(dif == closest)
    t1 = times[idx[0][0], idx[1][0]]

    if bg is not None:
        base_img_0 = np.array(im.open(bg))
        idxs = img < base_img_0
        base_img_0[idxs] = img[idxs]
    else:
        base_img_0 = 0 * img
        idxs = np.logical_or(times <= t1, times >= t2)
        base_img_0[idxs] = img[idxs]


    if annotate is True:
        start_time = get_start_time(pix_times_fname)

    time_step = 1000/fps

    if t2 is None:
        t2 = np.max(times.flatten())

    x_dim, y_dim = img.shape

    for t in range(t1, t2, time_step):
        base_img = np.copy(base_img_0)
        idxs = np.logical_and(times >= t-0*time_step, times <= t)
        base_img[idxs] = img[idxs]
        fname_out = '%s%06d%s' % (frame_prefix, t, '.png')
        img_out = im.fromarray(base_img)
        if annotate is True:
            time_diff = dt.timedelta(microseconds=1000*t)
            timestamp = start_time + time_diff
            timestamp = timestamp.strftime("%Y-%m-%d %H:%M:%S.%f UTC")
            draw = imd.Draw(img_out)
            font  = imf.truetype("/usr/share/matplotlib/mpl-data/fonts/ttf/STIXGeneral.ttf", 16)
            draw.text((250,5), timestamp, font=font, fill='#ffffff')
        img_out.save(fname_out)


if __name__ == "__main__":
    '''Save given time interval from peak-hold image as separate
    images.'''

    if len(sys.argv) < 6:
        print ''
        print 'Usage:\n'
        print '\t%s <max.png> <pixel_times.png> <background.png> <prefix> <t1,t2>\n' % sys.argv[0]
        print 'for example:\n'
        print '\t%s 2013-01-18_235308_UTC_60_s_max.png 2013-01-18_235308.110_UTC_pixel_times.png 2013-01-18_234904_UTC_60_s_max.png anim_ 5000,25000\n' % sys.argv[0]
    else:
        img_in = sys.argv[1]
        times_in = sys.argv[2]
        background = sys.argv[3]
        frame_prefix = sys.argv[4]
        t1, t2 = sys.argv[5].split(',')
        t1, t2 = int(t1), int(t2)

        sc2frames(img_in, times_in, t1=t1, t2=t2,frame_prefix=frame_prefix, 
                  bg=background, annotate=True)


# In linux, the command below can be used for compiling a video from
# the frames. Adjust file mask ("anim_*.png") and fps to suit your needs:
# mencoder mf://anim_*.png -mf fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=10125000:mbd=2:trell -oac copy -o video.avi
                                              

