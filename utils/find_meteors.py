#!/usr/bin/env python3

import os.path
import glob
import sys
import datetime as dt
import logging

from PIL import Image
import numpy as np

logging.basicConfig(level=logging.INFO, format='%(message)s')

LOGGER = logging.getLogger('find_meteors')

BIG_NUMBER = int(1e18)
AVE_MAX_RATIO_LIMIT = 0.5
DIST_LIMIT = 5**2
SIZE_LIMIT = 16
TRAVEL_LIMIT = 5
DURATION_LIMIT = 10
SPEED_LIMIT = 19
MIN_TIME_DIFF = 100

SAVE_DIR = "/tmp/"


def read_max(fname):
    """Read max image"""
    img = np.array(Image.open(fname))
    if len(img.shape) == 3:
        img = np.mean(img, 2)

    return img


def read_ave(fname):
    """Read sum image and return average."""
    img = np.array(Image.open(fname), dtype=np.float)

    r__ = img[:, :, 0]
    g__ = img[:, :, 1]
    b__ = img[:, :, 2]

    avg = r__ * 2**16 + g__ * 2**8 + b__
    avg /= avg[0, 0]

    return avg


def read_time(fname):
    """Convert time image to milliseconds.  Return also start time as
    datetime"""
    img = np.array(Image.open(fname), dtype=np.uint32)
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

    meteors = {}
    removed = {}
    camera = ""

    def __init__(self, max_fname, ave_fname, time_fname, mask=None):
        self.img_max = read_max(max_fname)
        self.img_ave = read_ave(ave_fname)
        self.times, self.start_time = read_time(time_fname)

        self.camera = os.path.basename(max_fname).split('_')[0]

        self.candidates = self.get_candidates()
        if mask is not None:
            self.candidates[mask] = False
        LOGGER.info("%d candidate pixels found" % self.candidates.sum())
        self.create()
        self.size_filter()
        self.join_candidates()
        self.speed_filter()

    def get_candidates(self):
        """Find meteor candidates."""
        LOGGER.info("Finding meteor candidates")
        if len(self.img_max.shape) == 3:
            ratio = self.img_ave / np.mean(self.img_max, 2)
        else:
            ratio = self.img_ave / self.img_max

        candidates = ratio < AVE_MAX_RATIO_LIMIT
        if candidates.sum() > 3000:
            candidates = ratio < np.mean(ratio) / 2.

        return candidates

    def get_unique_times(self):
        return np.sort(np.unique(self.times[np.where(self.candidates)]))

    def create(self):
        """Create initial clusters"""
        LOGGER.info("Create clusters")
        msec = self.times.copy()
        msec[np.invert(self.candidates)] = BIG_NUMBER

        unique_times = self.get_unique_times()
        for t__ in unique_times:
            y_idxs, x_idxs = np.where(msec == t__)
            for y__, x__ in zip(y_idxs, x_idxs):
                self._add_to_cluster(x__, y__, t__)

    def _add_to_cluster(self, x__, y__, t__):
        """Add points to a cluster"""
        clusters = self.meteors
        count = len(clusters)
        for key in clusters:
            x_idxs = np.array(clusters[key]['x'])
            y_idxs = np.array(clusters[key]['y'])
            dists = (x_idxs - x__)**2 + (y_idxs - y__)**2
            if np.min(dists) < DIST_LIMIT:
                self.__add(key, x__, y__, t__)
                return

        self.meteors[count] = {'x': [], 'y': [], 't': []}
        self.__add(count, x__, y__, t__)

    def __add(self, key, x__, y__, t__):
        """Add values to cluster *key*."""
        self.meteors[key]['x'].append(x__)
        self.meteors[key]['y'].append(y__)
        self.meteors[key]['t'].append(t__)

    def size_filter(self):
        """Filter clusters based on their size."""
        LOGGER.info("Filter clusters based on size")
        clusters = self.meteors
        valid = {}
        removed = {}
        for key in clusters:
            if len(clusters[key]['x']) > SIZE_LIMIT:
                valid[key] = {}
                for itm in clusters[key]:
                    valid[key][itm] = clusters[key][itm]
            else:
                removed[key] = {}
                for itm in clusters[key]:
                    removed[key][itm] = clusters[key][itm]

        self.meteors = valid
        self.removed = removed

    def speed_filter(self):
        """Apply speed filtering."""
        LOGGER.info("Filter clusters based on speed")
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

            # print distance, duration, speed
            if (distance > TRAVEL_LIMIT and duration < DURATION_LIMIT and
                    speed > SPEED_LIMIT):
                out[key] = {}
                for itm in clusters[key]:
                    out[key][itm] = clusters[key][itm]
            # else:
            #     lisaa self.removed:
            #         iin

        self.meteors = out

    def extend_clusters(self):
        """Extend clusters based on times at the beginning and end of
        automatic detection."""
        pass

    def join_candidates(self):
        """Join clusters that are clearly from the same event."""
        valid = self.meteors
        removed = self.removed
        LOGGER.info("Join %d clusters", len(removed))
        num_added = 0
        for key in removed:
            r_t = np.array(removed[key]['t'])
            r_x = removed[key]['x']
            r_y = removed[key]['y']

            for key in valid:
                v_t = valid[key]['t']
                # v_t_min = np.min(v_t)
                # v_t_max = np.max(v_t)
                v_x = valid[key]['x']
                v_y = valid[key]['y']

                for i in range(len(r_t)):
                    if r_t[i] == BIG_NUMBER:
                        continue
                    if np.min(np.abs(v_t - r_t[i])) > MIN_TIME_DIFF:
                        continue
                    dists = (v_x - r_x[i])**2 + (v_y - r_y[i])**2
                    if np.min(dists) < DIST_LIMIT:
                        self.__add(key, r_x[i], r_y[i], r_t[i])
                        # print "added to", key
                        r_t[i] = BIG_NUMBER
                        added_new = True
                        num_added += 1

                # if not added_new:
                #         break

        LOGGER.info("Joined %d pixels to other clusters", num_added)

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
            out_fname = os.path.join(SAVE_DIR, out_fname)
            x__ = self.meteors[key]['x']
            y__ = self.meteors[key]['y']
            with open(out_fname, 'w') as fid:
                fid.write("# %s\n" % start_time)
                fid.write("# relative time [s], x, y\n")
                for i in range(times.size):
                    fid.write("%.3f,%d,%d\n" % (times[i], x__[i], y__[i]))

    def _get_meteor_times(self, key):
        """Get start and relative times for meteors"""
        times = np.array(self.meteors[key]['t']) / 1000.
        time_min = np.min(times)
        start_time = self.start_time + dt.timedelta(seconds=int(time_min))
        start_time = start_time.strftime("%Y%m%d_%H%M%S.%f")
        times -= time_min

        return start_time, times

    def draw(self):
        """Draw detections over max stack"""
        if len(self.meteors) == 0:
            return
        shp = self.img_max.shape
        img = np.repeat(self.img_max, 3).reshape((shp[0], shp[1], 3)) / 255.
        for i, key in enumerate(self.meteors):
            x__ = np.array(self.meteors[key]['x'])
            y__ = np.array(self.meteors[key]['y'])
            img[y__, x__, :] = [1., 0., 0.]

        img *= 255
        img = Image.fromarray(img.astype(np.uint8))
        fname = "%s_%s.jpg" % (self.camera,
                               self.start_time.strftime("%Y%m%d_%H%M%S.%f"))
        img.save(os.path.join(SAVE_DIR, fname))


def main():
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

if __name__ == "__main__":
    main()
