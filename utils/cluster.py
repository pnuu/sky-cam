#!/usr/bin/env python

import os.path
import glob
import sys
import datetime as dt

from PIL import Image
import numpy as np
import matplotlib.pyplot as plt

import pdb


AVE_MAX_RATIO_LIMIT = 0.5
DIST_LIMIT = 5**2
SIZE_LIMIT = 15


def convert_ave(img):
    r__, g__, b__ = img.split()
    r__ = np.array(r__).astype(np.float)
    g__ = np.array(g__).astype(np.float)
    b__ = np.array(b__).astype(np.float)

    avg = r__ * 2**16 + g__ * 2**8 + b__
    avg /= avg[0, 0]

    return avg


def convert_time(img):
    r__, g__, b__ = img.split()
    r__ = np.array(r__).astype(np.uint32)
    g__ = np.array(g__).astype(np.uint32)
    b__ = np.array(b__).astype(np.uint32)

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
    for t__ in unique_times:
        y_idxs, x_idxs = np.where(msec == t__)
        for y__, x__ in zip(y_idxs, x_idxs):
            add_to_cluster(clusters, x__, y__, t__)

    out = {}
    for key in clusters:
        if len(clusters[key]['x']) > SIZE_LIMIT:
            out[key] = clusters[key]

    return out


def add_to_cluster(clusters, x__, y__, t__):
    """baz"""
    def add(clusters, key, x__, y__, t__):
        clusters[key]['x'].append(x__)
        clusters[key]['y'].append(y__)
        clusters[key]['t'].append(t__)

    count = len(clusters)
    for key in clusters:
        x_idxs = np.array(clusters[key]['x'])
        y_idxs = np.array(clusters[key]['y'])
        dists = (x_idxs - x__)**2 + (y_idxs - y__)**2
        if np.min(dists) < DIST_LIMIT:
            add(clusters, key, x__, y__, t__)
            return

    clusters[count] = {'x': [], 'y': [], 't': []}
    add(clusters, count, x__, y__, t__)


def main():
    """main()"""
    max_fname = sys.argv[3]
    ave_fname = sys.argv[2]
    time_fname = sys.argv[1]

    img_max = np.array(Image.open(max_fname))
    img_ave = convert_ave(Image.open(ave_fname))
    msec, start_time = convert_time(Image.open(time_fname))

    mask = find_candidates(img_max, img_ave)
    print np.sum(mask)
    unique_times = np.sort(np.unique(msec[np.where(mask)]))

    clusters = cluster(msec, unique_times, mask)

    draw(img_max, clusters)

    # pdb.set_trace()

    print len(clusters)


def draw(img_max, clusters):
    """"""
    plt.imshow(img_max, cmap='gray')
    # markers = ['r.', 'y.']
    for i, key in enumerate(clusters):
        x__ = np.array(clusters[key]['x'])
        y__ = np.array(clusters[key]['y'])
        plt.plot(x__, y__, '.')

    plt.show()


if __name__ == "__main__":
    main()
