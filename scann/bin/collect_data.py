#!/usr/bin/env python

import os.path
import glob
import sys
import datetime as dt
from shutil import copyfile

import yaml
from PIL import Image
import numpy as np

from trollsift import parse, compose

from scann import features as ft

PIXEL_FILE = "pixels.yaml"
TRAINING_FILE = "training_data.yaml"
TIME_FORMAT = "{time:%Y-%m-%d_%H%M%S}"
PATTERN_MAX = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_max.png"
PATTERN_AVE = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_ave24.png"
PATTERN_TIME = "{camera}_{time:%Y-%m-%d_%H%M%S}.???_UTC_pixel_times.png"

FEATURES = {'pix_max_val': ft.pix_max_val,
            'pix_avg_mean_ratio': ft.pix_avg_mean_ratio,
            'pix_max_mean_ratio': ft.pix_max_mean_ratio,
            'pix_avg_max_ratio': ft.pix_avg_max_ratio,
            'flash': ft.flash}

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
    r__ = np.array(r__).astype(np.uint32)
    g__ = np.array(g__).astype(np.uint32)
    b__ = np.array(b__).astype(np.uint32)

    start_time = 2**16 * (r__[0][0] * 2**16 + g__[0][0] * 2**8 + b__[0][0]) + \
        (r__[0][1] * 2**16 + g__[0][1] * 2**8 + b__[0][1]) + \
        (r__[0][2] * 2**16 + g__[0][2] * 2**8 + b__[0][2]) / 1000.

    start_time = dt.datetime.utcfromtimestamp(start_time)

    msec = (r__ * 2**16 + g__ * 2**8 + b__)

    return msec, start_time


def collect_data(training_dir):
    """Collect data from the given directory."""
    data = {}
    pix_data = read_yaml(training_dir)
    for date in pix_data:
        print date
        pattern = "*" + compose(TIME_FORMAT, {'time': date}) + "*max.png"
        max_file = glob.glob(os.path.join(training_dir, pattern))[0]
        pattern = "*" + compose(TIME_FORMAT, {'time': date}) + "*ave24.png"
        avg_file = glob.glob(os.path.join(training_dir, pattern))[0]
        pattern = "*" + compose(TIME_FORMAT, {'time': date}) + "*times.png"
        time_file = glob.glob(os.path.join(training_dir, pattern))[0]

        # Read max image
        img_max = np.array(Image.open(max_file)).astype(np.float)
        # Read average stack
        img_avg = convert_ave(Image.open(avg_file))
        # Calculate max/avg ratio
        # img_ratio = img_max / img_avg
        # Read time image
        img_time, start_time = convert_time(Image.open(time_file))

        for cat in pix_data[date]:
            print "\t", cat
            if cat not in data:
                data[cat] = {}

            for feat in FEATURES:
                print "\t\t", feat
                func = FEATURES[feat]
                res = func(img_max, img_avg, img_time)
                for x__, y__ in pix_data[date][cat]:
                    if feat not in data[cat]:
                        data[cat][feat] = []
                    data[cat][feat].append(float(res[y__, x__]))
                

    return data


def save_data(data, training_dir):
    """Save data to the given dir"""
    out_file = os.path.join(training_dir, TRAINING_FILE)
    try:
        copyfile(out_file, out_file + '.bak')
    except IOError:
        pass

    with open(out_file, 'w') as fid:
        yaml.dump(data, stream=fid)

    print "Category data saved to", out_file



def main():
    """main()"""
    training_dir = sys.argv[1]
    data = collect_data(training_dir)
    import pdb
    pdb.set_trace()
    save_data(data, training_dir)

if __name__ == "__main__":
    main()
