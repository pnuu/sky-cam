#!/usr/bin/python

import Image as im
from PIL import ImageDraw as imd
from PIL import ImageFont as imf
import sys
import numpy as np
import datetime as dt
import time
from glob import glob
from collections import deque
import os

sc_dir = '/home/pnuu/arkisto/sky-cam/watec/'
out_file = '/home/pnuu/Pictures/sky-cam/perkkaa_latest_ave.jpg'
mask = '/home/pnuu/Pictures/perkkaa_mask.png'
num_in_long = 5
pause_length = 5
images_needed = num_in_long + pause_length + 1


def tail(filename, n=10):
    'Return the last n lines of a file'
    return deque(open(filename), n)


def img_ave(img):
    r, g, b = img.split()
    r = np.array(r).astype('float64')
    g = np.array(g).astype('float64')
    b = np.array(b).astype('float64')
    
    avg = r * 2**16 + g * 2**8 + b
    avg /= avg[0,0]

    return avg


def stretch_img(img, bits, mask=None):

    if mask is None:
        mask = mask == mask

    img2 = img[mask].ravel()
    img_std = np.std(img2)
    img_min = np.mean(img2) - 5*img_std
    img_max = np.mean(img2) + 10*img_std

    img -= img_min
    img /= (img_max - img_min)
    img *= (2**bits - 1)
    img[img < 0] = 0
    img[img > 2**bits - 1] = 2**bits -1

    if bits == 8:
        return img.astype('uint8')
    else:
        return img.astype('uint16')


if __name__ == '__main__':

    mask = im.open(mask)
    mask.load()
    mask = np.array(mask)
    mask = mask == 0

    today = dt.datetime.utcfromtimestamp(time.time())
    yesterday = today - dt.timedelta(days=1)

    files_today = sc_dir + '%d/%02d/%02d/ave/*ave24*png' % (today.year, 
                                                            today.month, 
                                                            today.day)
    files_today = glob(files_today)
    files_today.sort()

    time_latest = os.path.getmtime(files_today[-1])
    time_now = time.time()

    if time_now - time_latest > 2*60:
        sys.exit()

    if len(files_today) < images_needed:
        files_yesterday = sc_dir + \
            '%d/%02d/%02d/ave/*ave24*png' % (yesterday.year, 
                                             yesterday.month, 
                                             yesterday.day)
        files_yesterday = glob(files_yesterday)
        files_yesterday.sort()
        files = files_yesterday[-1*(images_needed-len(files_today)):-1]
        files.append(files_yesterday[-1])
        for f in files_today:
            files.append(f)
        files.sort()
    else:
        files = files_today[-1*images_needed-1:-1]
        files.append(files_today[-1])

    long_avg = 0

    # Form long average
    for i in range(0, num_in_long):
        img = im.open(files[i])
        img.load()
        long_avg += img_ave(img)
    long_avg /= num_in_long

    # Subtract long average from latest image, scale and save
    img = im.open(files[-1])
    img.load()
    img = img_ave(img)
    x, y = img.shape
    p = np.polyfit(img.ravel(), long_avg.ravel(), 1)
    img = np.polyval(p, img.ravel()) - long_avg.ravel()
    img = img.reshape((x,y))
#    img -= long_avg
    # Stretch image and convert it to uint8 array
    img = stretch_img(img, 8, mask=mask)
    w, h = img.shape
    img = im.fromarray(img, mode='L')
    # Annotate image
    timestamp = files[-1].split('/')[-1].split('_')
    date_parts = timestamp[1].split('-')
    time_parts = timestamp[2]
    timestamp = date_parts[2] + '.' + date_parts[1] + '.' + date_parts[0] + ' '
    timestamp += time_parts[0:2] + '.' + time_parts[2:4] + '.' + time_parts[4:6]
    timestamp += ' UTC + 60 s '
#    logs = glob(sc_dir + '*/*/*log')
#    logs.sort()
#    log = logs[-1]
#    row = tail(log, n=1)[0]
#    row = row.split(',')
#    timestamp = row[2] + '.' + row[1] + '.' + row[0] + ' ' + row[3] + '.' + row[4] + '.' + row[5][0:2] + ' UTC + 60 s'

    draw = imd.Draw(img)
    font  = imf.truetype("/usr/share/matplotlib/mpl-data/fonts/ttf/STIXGeneral.ttf", 20)
    draw.text((30,545), timestamp, font=font, fill='#ffffff')
    img.save(out_file)

