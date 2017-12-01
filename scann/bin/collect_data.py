#!/usr/bin/env python

import os.path
import glob
import sys
import datetime as dt

import yaml
from PIL import Image
import numpy as np

from trollsift import parse, compose

PIXEL_FILE = "pixels.yaml"
TIME_FORMAT = "{time:%Y-%m-%d_%H%M%S}"
PATTERN_MAX = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_max.png"
PATTERN_AVE = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_ave24.png"
PATTERN_TIME = "{camera}_{time:%Y-%m-%d_%H%M%S}.???_UTC_pixel_times.png"

def read_yaml(training_dir):
    """Get data from from file, or return empty dictionary"""
    try:
        with open(os.path.join(training_dir, PIXEL_FILE), 'r') as fid:
            data = yaml.load(fid)
    except IOError:
        return {}

    if data is None:
        data = {}

    return data


def get_avetime_file(path, pattern, max_file):
    """Get ave and time files from the given directory"""
    parts = parse(PATTERN_MAX, os.path.basename(max_file))

    try:
        fname = glob.glob(os.path.join(path, compose(pattern, parts)))[0]
    except IndexError:
        return None

    if not os.path.exists(fname):
        return None

    return fname


def files_from_dir(base_dir):
    """Get sets of files from the given directory"""
    dir_files = []
    max_files = glob.glob(os.path.join(base_dir, '*max.png'))

    for max_file in max_files:
        ave_file = get_avetime_file(base_dir, PATTERN_AVE, max_file)
        time_file = get_avetime_file(base_dir, PATTERN_TIME, max_file)
        if ave_file is None or time_file is None:
            continue
        dir_files.append({'max': max_file, 'ave': ave_file, 'time': time_file})

    return dir_files


def convert_ave(img):
    r__, g__, b__ = img.split()
    r__ = np.array(r__).astype(np.float)
    g__ = np.array(g__).astype(np.float)
    b__ = np.array(b__).astype(np.float)

    avg = r__ * 2**16 + g__ * 2**8 + b__
    avg /= avg[0,0]

    return avg

def convert_time(img):
    r__, g__, b__ = img.split()
    r__ = np.array(r__).astype(np.float)
    g__ = np.array(g__).astype(np.float)
    b__ = np.array(b__).astype(np.float)

    start_time = 2**16 * (r__[0][0] * 2**16 + g__[0][0] * 2**8 + b__[0][0]) + \
        (r__[0][1] * 2**16 + g__[0][1] * 2**8 + b__[0][1]) + \
        (r__[0][2] * 2**16 + g__[0][2] * 2**8 + b__[0][2])/1000.

    start_time = dt.datetime.utcfromtimestamp(start_time)

    sec = .001 * (r__ * 2**16 + g__ * 2**8 + b__)

    return sec, start_time


def collect_data(training_dir):
    """Collect data from the given directory."""
    data = {}
    pix_data = read_yaml(training_dir)
    for date in pix_data:
        max_file = glob.glob(os.path.join(training_dir,
                                          "*" + compose(TIME_FORMAT,
                                                        date) + "*max.png"))
        avg_file = glob.glob(os.path.join(training_dir,
                                          "*" + compose(TIME_FORMAT,
                                                        date) + "*ave24.png"))
        time_file = glob.glob(os.path.join(training_dir,
                                           "*" + compose(TIME_FORMAT,
                                                         date) + "*times.png"))

        # Read max image
        img_max = np.array(Image.open(max_file)).astype(np.float)
        # Read average stack
        img_avg = convert_ave(Image.open(avg_file))
        # Calculate max/avg ratio
        img_ratio = img_max / img_avg
        # Read time image
        img_time, start_time = convert_time(Image.open(time_file))

        for cat in data[date]:
            if cat not in data:
                data[cat] = init_feature_dict()

            for x__, y__ in pix_data[cat]:
                for feat in data[cat]:
                    res = get_feature_data(feat, max_file, avg_file, time_file)
                    data[cat][feat] += res
                

    return data


def init_feature_dict():
    """Initialize feture dictionary"""
    return {}


def save_data(data, training_dir):
    """Save data to the given dir"""
    pass


def main():
    """main()"""
    training_dir = sys.argv[1]
    data = collect_data(training_dir)
    save_data(data, training_dir)

if __name__ == "__main__":
    main()
