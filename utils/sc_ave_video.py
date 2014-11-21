#!/usr/bin/python

import Image as im
from PIL import ImageDraw as imd
from PIL import ImageFont as imf
import sys
import os
import numpy as np
import datetime as dt
import time
from glob import glob

sc_dir = '/home/pnuu/arkisto/sky-cam/'
out_dir = '/home/pnuu/arkisto/sky-cam/videot/'
watec_mask = '/home/pnuu/Pictures/perkkaa_mask.png'
ivalo_mask = '/home/pnuu/Pictures/ivalo_mask.png'

def img_ave(img):
    r, g, b = img.split()
    r = np.array(r).astype('float64')
    g = np.array(g).astype('float64')
    b = np.array(b).astype('float64')
    
    avg = r * 2**16 + g * 2**8 + b
    avg /= avg[0,0]

    return avg


def histeq(img, nbr_bins=256):

   #get image histogram
   imhist, bins = np.histogram(img.flatten(), nbr_bins, normed=True)
   cdf = imhist.cumsum() #cumulative distribution function
   cdf = 255 * cdf / cdf[-1] #normalize

   #use linear interpolation of cdf to find new pixel values
   img2 = np.interp(img.flatten(), bins[:-1], cdf)

   return img2.reshape(img.shape) #, cdf


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

    num_in_long = int(sys.argv[1])
    pause_length = int(sys.argv[2])
    camera = sys.argv[3]
    day_diff = int(sys.argv[4])

    if camera == 'watec':
        mask = watec_mask
    else:
        mask = ivalo_mask

    mask = im.open(mask)
    mask.load()
    mask = np.array(mask)
    mask = mask == 0

    day = dt.datetime.utcfromtimestamp(time.time()) - dt.timedelta(days=day_diff)

    directory = sc_dir + '%s/%d/%02d/%02d/ave/' % (camera, 
                                                   day.year, 
                                                   day.month, 
                                                   day.day)
    print directory
    os.chdir(directory)

    images = glob(camera+'*ave24*png')
    images.sort()

    long_avg = 0

    # Form long average
    for i in range(0, num_in_long):
        try:
            img = im.open(images[i])
            img.load()
	except:
            print "Bad image: " + images[i]
            continue
        long_avg += img_ave(img)
    long_avg /= num_in_long

    # Subtract long average from all images, scale and save
    for i in range(num_in_long+pause_length, len(images)):
        #foo = im.fromarray(long_avg.astype('uint8'), mode='L')
        #foo.save('long_'+images[i])
        try:
            img = im.open(images[i])
            img.load()
        except:
            print "Bad image: " + images[i]
            continue
        img = img_ave(img)
        x, y = img.shape
        p = np.polyfit(img.ravel(), long_avg.ravel(), 1)
        img = np.polyval(p, img.ravel()) - long_avg.ravel()
        img = img.reshape((x,y))
        #img = img - long_avg
        #fname = images[i][0:-4] + '.H5'
        #fid = h5py.File(fname, 'w')
        #fid['data'] = img
        #fid.close()
        # Stretch image and convert it to uint8 array
        img = stretch_img(img, 8, mask=mask)
        w, h = img.shape
        img = im.fromarray(img, mode='L')
        fname = 'mod_' + images[i]

        # Annotate image
        timestamp = images[i].split('/')[-1].split('_')
        date_parts = timestamp[1].split('-')
        time_parts = timestamp[2]
        timestamp = date_parts[2] + '.' + date_parts[1] + '.' + date_parts[0] + ' '
        timestamp += time_parts[0:2] + '.' + time_parts[2:4] + '.' + time_parts[4:6]
        timestamp += ' UTC + 60 s '
        draw = imd.Draw(img)
        font  = imf.truetype("/usr/share/matplotlib/mpl-data/fonts/ttf/STIXGeneral.ttf", 20)
        draw.text((30,550), timestamp, font=font, fill='#ffffff')

        img.save(fname)
        try:
            img = im.open(images[i-num_in_long-pause_length])
            img.load()
        except:
            print "Bad image: " + images[i-num_in_long-pause_length]
            continue
        long_avg -= img_ave(img)/num_in_long
        try:
            img = im.open(images[i-pause_length])
            img.load()
        except:
            print "Bad image: " + images[i-num_in_long-pause_length]
            continue
        long_avg += img_ave(img)/num_in_long

    video_fname = out_dir + '%s_%d%02d%02d_timelapse.avi' % (camera, 
                                                             day.year, 
                                                             day.month, 
                                                             day.day)
    print video_fname
    mencoder_cmd = '/usr/bin/mencoder mf://mod*png -mf fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=10125000:mbd=2:trell -oac copy -o %s' % (video_fname)
    os.system(mencoder_cmd)

