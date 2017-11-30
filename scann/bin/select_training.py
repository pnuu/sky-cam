#!/usr/bin/env python

"""Select points from image and assign them to categories."""

import sys
import datetime as dt
from random import randrange
from datetime import timedelta
from shutil import copyfile
import os.path
import glob

import matplotlib.pyplot as plt
from PIL import Image
import numpy as np
import yaml

from trollsift import parse, compose

START = dt.datetime(2014, 3, 1)

OUT_FILE = "pixels.yaml"
PATTERN_MAX = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_max.png"
PATTERN_AVE = "{camera}_{time:%Y-%m-%d_%H%M%S}_UTC_{length:d}_s_ave24.png"
PATTERN_TIME = "{camera}_{time:%Y-%m-%d_%H%M%S}.???_UTC_pixel_times.png"


def random_date(start=START, end=dt.datetime.utcnow()):
    """Get a random datetime
    """
    delta = end - start
    int_delta = int(delta.total_seconds())
    random_second = randrange(int_delta)

    return start + timedelta(seconds=random_second)


def get_start_date(base_dir):
    """Get earliest date in base_dir"""
    date = get_dir_date(base_dir, 0)

    return max(START, date)


def get_end_date(base_dir):
    """Get earliest date in base_dir"""
    date = get_dir_date(base_dir, -1)

    return min(dt.datetime.utcnow(), date)


def get_dir_date(base_dir, idx):
    """Get date of the latest (idx = -1) or earliest (idx = 0) date in
    the given base directory."""
    year = os.path.split(sorted(glob.glob(os.path.join(base_dir,
                                                       '????')))[idx])[-1]
    month = os.path.split(sorted(glob.glob(os.path.join(base_dir, year,
                                                       '??')))[idx])[-1]
    day = os.path.split(sorted(glob.glob(os.path.join(base_dir, year, month,
                                                       '??')))[idx])[-1]

    return dt.datetime(int(year), int(month), int(day))
    

def read_yaml(training_dir):
    """Get data from from file, or return empty dictionary"""
    try:
        with open(os.path.join(training_dir, OUT_FILE), 'r') as fid:
            data = yaml.load(fid)
    except IOError:
        return {}

    if data is None:
        data = {}

    return data


def get_random_fileset(base_dir):
    """Get a random set of max/ave/pixel_times files."""

    while True:
        date = random_date(start=get_start_date(base_dir),
                           end=get_end_date(base_dir))
        day_path = os.path.join(base_dir, '%d' % date.year,
                                '%02d' % date.month, '%02d' % date.day)
        max_files = glob.glob(os.path.join(day_path, 'max', '*.png'))

        try:
            idx = randrange(len(max_files))
            max_file = max_files[idx]
        except ValueError:
            continue

        ave_file = get_avetime_file(os.path.join(day_path, 'ave'),
                                    PATTERN_AVE,
                                    max_file)
        time_file = get_avetime_file(os.path.join(day_path, 'ave'),
                                    PATTERN_TIME,
                                    max_file)

        if ave_file is None or time_file is None:
            continue

        break

    return {'max': max_file, 'ave': ave_file, 'time': time_file}


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


def run(base_dir, training_dir):
    """Run pixel selection"""
    data = read_yaml(training_dir)

    if os.path.abspath(base_dir) == os.path.abspath(training_dir):
        dir_files = files_from_dir(base_dir)
    else:
        dir_files = []

    i = 0
    while True:
        # Get a set of files
        if len(dir_files) == 0:
            fnames = get_random_fileset(base_dir)
        else:
            try:
                fnames = dir_files[i]
            except IndexError:
                break
        i += 1
        date = parse(PATTERN_MAX, fnames['max'])['time']
        # Read maximum stack
        img = np.array(Image.open(fnames['max']))
        # Collect points from image for and associate them to a category
        while True:
            plt.imshow(img, interpolation='none')
            plt.title(str(date))
            pts = plt.ginput(timeout=0, n=0)
            if len(pts) > 0:
                if date not in data:
                    data[date] = {}
                cat = raw_input("Give category: ")
                data[date][cat] = [(int(round(y)), int(round(x))) for
                                   y, x in pts]

                if raw_input("Continue with this image? [Y/n]: ") in ["", "y",
                                                                      "Y"]:
                    continue
                else:
                    break
            else:
                break
        if date in data:
            for key in fnames:
                try:
                    copyfile(fnames[key], training_dir)
                except:
                    pass

        if raw_input("Next image? [Y/n]: ") in ["", "y", "Y"]:
            continue
        else:
            break

    print "Images used:", i
    save_yaml(data, training_dir)


def save_yaml(data, training_dir):
    """Save data to training dir"""
    out_file = os.path.join(training_dir, OUT_FILE)
    try:
        copyfile(out_file, out_file + '.bak')
    except IOError:
        pass

    with open(out_file, 'w') as fid:
        yaml.dump(data, stream=fid)

    print "Category data saved to", out_file


def main():
    """main()"""
    base_dir = sys.argv[1]
    training_dir = sys.argv[2]

    run(base_dir, training_dir)


if __name__ == "__main__":
    # print(get_random_fileset('/home/pnuu/mnt/arkisto3/sky-cam/watec'))
    main()
