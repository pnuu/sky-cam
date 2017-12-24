#!/usr/bin/env python

import os.path
import glob
import sys
import datetime as dt

from PIL import Image
import numpy as np
import matplotlib.pyplot as plt

import pdb
from numba import jit

AVE_MAX_RATIO_LIMIT = 0.5
DIST_LIMIT = 5**2
SIZE_LIMIT = 21
TRAVEL_LIMIT = 5
DURATION_LIMIT = 10
SPEED_LIMIT = 19


def convert_ave(img):
    """Convert sum image to average."""
    r__ = img[:, :, 0]
    g__ = img[:, :, 1]
    b__ = img[:, :, 2]

    avg = r__ * 2**16 + g__ * 2**8 + b__
    avg /= avg[0, 0]

    return avg


def convert_time(img):
    """Convert time image to milliseconds.  Return also start time as
    datetime"""
    r__ = img[:, :, 0]
    g__ = img[:, :, 1]
    b__ = img[:, :, 2]

    start_time = 2**16 * (r__[0][0] * 2**16 + g__[0][0] * 2**8 + b__[0][0]) + \
        (r__[0][1] * 2**16 + g__[0][1] * 2**8 + b__[0][1]) + \
        (r__[0][2] * 2**16 + g__[0][2] * 2**8 + b__[0][2]) / 1000.

    start_time = dt.datetime.utcfromtimestamp(start_time)

    msec = (r__ * 2**16 + g__ * 2**8 + b__)

    return msec, start_time


def find_candidates(img_max, img_ave):
    """foo"""
    if len(img_max.shape) == 3:
        ratio = img_ave / np.mean(img_max, 2)
    else:
        ratio = img_ave / img_max

    mask = ratio < AVE_MAX_RATIO_LIMIT

    return mask


def cluster(msec_times, unique_times, mask):
    """bar"""
    msec = msec_times.copy()
    msec[np.invert(mask)] = int(1e18)

    clusters = {}
    create_clusters(clusters, unique_times, msec)

    clusters = size_filter_clusters(clusters)

    return clusters


def create_clusters(clusters, unique_times, msec):
    """"""
    for t__ in unique_times:
        y_idxs, x_idxs = np.where(msec == t__)
        for y__, x__ in zip(y_idxs, x_idxs):
            add_to_cluster(clusters, x__, y__, t__)


def size_filter_clusters(clusters):
    """"""
    out = {}
    for key in clusters:
        if len(clusters[key]['x']) > SIZE_LIMIT:
            out[key] = {}
            for itm in clusters[key]:
                out[key][itm] = np.array(clusters[key][itm])
    return out


def add_to_cluster(clusters, x__, y__, t__):
    """baz"""
    count = len(clusters)
    for key in clusters:
        x_idxs = np.array(clusters[key]['x'])
        y_idxs = np.array(clusters[key]['y'])
        dists = (x_idxs - x__)**2 + (y_idxs - y__)**2
        if np.min(dists) < DIST_LIMIT:
            __add(clusters, key, x__, y__, t__)
            return

    clusters[count] = {'x': [], 'y': [], 't': []}
    __add(clusters, count, x__, y__, t__)


def __add(clusters, key, x__, y__, t__):
    clusters[key]['x'].append(x__)
    clusters[key]['y'].append(y__)
    clusters[key]['t'].append(t__)


def main():
    """main()"""
    max_fname = sys.argv[3]
    ave_fname = sys.argv[2]
    time_fname = sys.argv[1]

    img_max = np.array(Image.open(max_fname))
    if len(img_max.shape) == 3:
        img_max = np.mean(img_max, 2)
    img_ave = convert_ave(np.array(Image.open(ave_fname), dtype=np.float))
    msec, start_time = convert_time(np.array(Image.open(time_fname),
                                             dtype=np.uint32))

    mask = find_candidates(img_max, img_ave)
    print np.sum(mask)
    unique_times = np.sort(np.unique(msec[np.where(mask)]))

    clusters = cluster(msec, unique_times, mask)
    clusters = speed_filter(clusters)
    draw(img_max, clusters)

    # pdb.set_trace()

    # print len(clusters.clusters)


def draw(img_max, clusters):
    """"""
    plt.imshow(img_max, cmap='gray', interpolation='none')
    # markers = ['r.', 'y.']
    for i, key in enumerate(clusters):
        x__ = np.array(clusters[key]['x'])
        y__ = np.array(clusters[key]['y'])
        plt.plot(x__, y__, '.')

    plt.show()


def speed_filter(clusters):
    """"""
    out = {}
    for key in clusters:
        min_idx = np.argmin(clusters[key]['t'])
        max_idx = np.argmax(clusters[key]['t'])
        min_t = clusters[key]['t'][min_idx]
        min_x = clusters[key]['x'][min_idx]
        min_y = clusters[key]['y'][min_idx]
        max_t = clusters[key]['t'][max_idx]
        max_x = clusters[key]['x'][max_idx]
        max_y = clusters[key]['y'][max_idx]

        distance = np.sqrt((max_x - min_x)**2 + (max_y - min_y)**2)
        duration = (max_t - min_t) / 1000.
        speed = distance / duration

        print distance, duration, speed

        if (distance > TRAVEL_LIMIT and duration < DURATION_LIMIT and
                speed > SPEED_LIMIT):
            out[key] = {}
            for itm in clusters[key]:
                out[key][itm] = np.array(clusters[key][itm])

        # Remove from clusters if:
        # - distance < 5
        # - duration > 10
        # - speed < 19

    return out


if __name__ == "__main__":
    main()
