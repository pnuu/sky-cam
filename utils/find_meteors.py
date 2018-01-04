#!/usr/bin/env python3

import os.path
import glob
import sys
import datetime as dt
import logging

from PIL import Image
import numpy as np

from skimage.measure import label
from skimage.morphology import closing, square, remove_small_objects
from skimage.color import label2rgb

logging.basicConfig(level=logging.INFO, format='%(message)s')

LOGGER = logging.getLogger('find_meteors')

"""
TODO:
- save also transients
- draw transients in blue
- add "meteor"/"transient" to the CSV filename
"""

BIG_NUMBER = int(1e18)
AVE_MAX_RATIO_LIMIT = 0.5
DIST_LIMIT = 5
SIZE_LIMIT = 16
TRAVEL_LIMIT = 5
DURATION_LIMIT_MIN = 0.1
DURATION_LIMIT_MAX = 10
SPEED_LIMIT = 19
MIN_TIME_DIFF = 100

SAVE_DIR = "/tmp/"


def read_max(fname):
    """Read max image"""
    img = np.array(Image.open(fname), dtype=np.float32)
    if len(img.shape) == 3:
        img = np.mean(img, 2)

    return img


def read_ave(fname):
    """Read sum image and return average."""
    img = np.array(Image.open(fname), dtype=np.float32)

    r__ = img[:, :, 0]
    g__ = img[:, :, 1]
    b__ = img[:, :, 2]

    avg = r__ * 2**16 + g__ * 2**8 + b__
    avg /= avg[0, 0]

    return avg


def read_time(fname):
    """Convert time image to milliseconds.  Return also start time as
    datetime"""
    img = np.array(Image.open(fname), dtype=np.float32)
    r__ = img[:, :, 0]
    g__ = img[:, :, 1]
    b__ = img[:, :, 2]

    start_time = 2**16 * (r__[0][0] * 2**16 + g__[0][0] * 2**8 + b__[0][0]) + \
        (r__[0][1] * 2**16 + g__[0][1] * 2**8 + b__[0][1]) + \
        (r__[0][2] * 2**16 + g__[0][2] * 2**8 + b__[0][2]) / 1000.

    start_time = dt.datetime.utcfromtimestamp(start_time)

    msec = (r__ * 2**16 + g__ * 2**8 + b__)

    return msec, start_time


class MeteorDetect(object):

    """Class for finding meteor streaks from sky-cam data."""

    camera = ""
    save_dir = SAVE_DIR

    def __init__(self, max_fname, ave_fname, time_fname,
                 mask=None, save_dir=SAVE_DIR):
        self.meteors = {}
        self.img_max = read_max(max_fname)
        self.img_ave = read_ave(ave_fname)
        self.times, self.start_time = read_time(time_fname)

        self.camera = os.path.basename(max_fname).split('_')[0]

        self.candidates = self.get_candidates()
        if mask is not None:
            self.candidates[mask] = False
        LOGGER.debug("%d candidate pixels found", self.candidates.sum())

        self.save_dir = save_dir

        self.create()
        self.speed_filter()

    def get_candidates(self):
        """Find meteor candidates."""
        LOGGER.debug("Finding meteor candidates")
        if len(self.img_max.shape) == 3:
            ratio = self.img_ave / np.mean(self.img_max, 2)
        else:
            ratio = self.img_ave / self.img_max

        candidates = ratio < AVE_MAX_RATIO_LIMIT

        return candidates

    def create(self):
        """Create initial clusters"""
        LOGGER.debug("Segment image")
        closed = closing(self.candidates, square(DIST_LIMIT))
        cleaned = remove_small_objects(closed, min_size=SIZE_LIMIT)
        labels, num = label(cleaned, background=0, return_num=True)
        for i in range(1, num + 1):
            y_idxs, x_idxs = np.where(labels == i)
            times = self.times[y_idxs, x_idxs]
            self.meteors[i] = {'x': x_idxs, 'y': y_idxs, 't': times}

    def speed_filter(self):
        """Apply speed filtering."""
        LOGGER.debug("Filter clusters based on speed")
        clusters = self.meteors
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
            duration = .001 * (max_t - min_t)
            speed = distance / duration

            # TODO: add transient category
            if (distance > TRAVEL_LIMIT and duration < DURATION_LIMIT_MAX and
                    duration > DURATION_LIMIT_MIN and speed > SPEED_LIMIT):
                out[key] = {}
                for itm in clusters[key]:
                    out[key][itm] = clusters[key][itm]

        self.meteors = out

    def extend_clusters(self):
        """Extend clusters based on times at the beginning and end of
        automatic detection."""
        pass

    def print_meteors(self):
        """Print meteor data"""
        for key in self.meteors:
            start_time, times = self._get_meteor_times(key)
            x__ = self.meteors[key]['x']
            y__ = self.meteors[key]['y']
            print("#", start_time)
            print("# relative time [s], x, y")
            for i in range(times.size):
                print(times[i], x__[i], y__[i])

    def save_meteors(self):
        """Save meteors"""
        for key in self.meteors:
            start_time, times = self._get_meteor_times(key)
            out_fname = "%s_%s.csv" % (self.camera, start_time)
            out_fname = os.path.join(self.save_dir, out_fname)
            idxs = np.argsort(times)
            times = times[idxs]
            x__ = self.meteors[key]['x'][idxs]
            y__ = self.meteors[key]['y'][idxs]
            LOGGER.info("Saving meteor: %s", out_fname)
            with open(out_fname, 'w') as fid:
                fid.write("# Start time: %s\n" % start_time)
                fid.write("# Time since start [s], x, y\n")
                for i in range(times.size):
                    fid.write("%.3f,%d,%d\n" % (times[i], x__[i], y__[i]))

    def _get_meteor_times(self, key):
        """Get start and relative times for meteors"""
        times = np.array(self.meteors[key]['t']) / 1000.
        time_min = np.min(times)
        start_time = self.start_time + dt.timedelta(seconds=float(time_min))
        start_time = start_time.strftime("%Y%m%d_%H%M%S.%f")
        times -= time_min

        return start_time, times

    def draw(self):
        """Draw detections over max stack"""
        if len(self.meteors) == 0:
            return
        shp = self.img_max.shape
        img = np.repeat(self.img_max, 3).reshape((shp[0], shp[1], 3)) / 255.
        for key in self.meteors:
            x__ = np.array(self.meteors[key]['x'])
            y__ = np.array(self.meteors[key]['y'])
            img[y__, x__, :] = [1., 0., 0.]

        img *= 255
        img = Image.fromarray(img.astype(np.uint8))
        fname = "%s_%s.jpg" % (self.camera,
                               self.start_time.strftime("%Y%m%d_%H%M%S.%f"))
        fname = os.path.join(self.save_dir, fname)
        img.save(fname)
        LOGGER.info("Saved preview image: %s", fname)


def manual_main():
    """main()"""
    max_fname = sys.argv[3]
    ave_fname = sys.argv[2]
    time_fname = sys.argv[1]
    try:
        mask_fname = sys.argv[4]
        mask = np.array(Image.open(mask_fname)) == 255
    except IndexError:
        mask = None

    meteors = MeteorDetect(max_fname, ave_fname, time_fname, mask=mask)
    # meteors.print_meteors()
    meteors.save_meteors()
    meteors.draw()


def cron_main():
    """Main() for cron usage"""
    base_dir = sys.argv[1]
    save_dir = sys.argv[2]
    try:
        mask_fname = sys.argv[3]
        mask = np.array(Image.open(mask_fname)) == 255
    except IndexError:
        mask = None

    max_fnames = glob.glob(os.path.join(base_dir, "max", "*max.png"))
    for max_fname in max_fnames:
        tstr = os.path.basename(max_fname).split('_')[2]

        try:
            time_fname = glob.glob(os.path.join(base_dir, "pixel_times",
                                                "*" + tstr + "*.png"))[0]
            ave_fnames = glob.glob(os.path.join(base_dir, "ave",
                                                "*" + tstr + "*.png"))
            ave_fname = ave_fnames[0]
            if os.path.basename(ave_fname).startswith("mod"):
                ave_fname = ave_fnames[1]
        except IndexError:
            continue

        meteors = MeteorDetect(max_fname, ave_fname, time_fname,
                               mask=mask, save_dir=save_dir)
        meteors.save_meteors()
        meteors.draw()


if __name__ == "__main__":
    # manual_main()
    cron_main()
