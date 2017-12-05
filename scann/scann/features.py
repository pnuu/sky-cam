import numpy as np


def pix_max_val(img_max, img_avg, img_time):
    """Return maximum value at the defined pixel."""
    return img_max / 255.


def pix_avg_mean_ratio(img_max, img_avg, img_time):
    """Return the ratio avg[y, x]/mean(avg)."""
    return img_avg / np.mean(img_avg)


def pix_max_mean_ratio(img_max, img_avg, img_time):
    """Return the ratio max[y, x]/mean(max)."""
    return img_max / np.mean(img_max)


def pix_avg_max_ratio(img_max, img_avg, img_time):
    """Return the ratio avg[y, x]/max[y, x]."""
    return img_avg / img_max


def flash(img_max, img_avg, img_time):
    """Return 0 if surroundings doesn't look like a transient flash,
    or 1 if it looks like one."""

    one_up = flash_moved_up(img_time, shift=1)
    one_down = flash_moved_down(img_time, shift=1)
    two_up = flash_moved_up(img_time, shift=2)
    two_down = flash_moved_down(img_time, shift=2)

    # True in two up/down AND False in one up/down -> flash
    one_up_down = np.logical_and(one_up, one_down)
    two_up_down = np.logical_and(two_up, two_down)

    return np.logical_and(np.invert(one_up_down), two_up_down)

    # if (((img_time == img_time[y__-2, x__-2:x__+3]).sum() > 0 or
    #     (img_time == img_time[y__+2, x__-2:x__+3]).sum() > 0) and
    #    ((img_time == img_time[y__-1, x__-2:x__+3]).sum() == 0 and
    #     (img_time == img_time[y__+1, x__-2:x__+3]).sum() == 0)):
    #    return 1.
    # else:
    #    return 0.


def flash_moved_up(img_time, shift):
    """Get subset for flash: shift one row up"""
    y_shp, x_shp = img_time.shape

    # Image shifted up
    up_ = img_time.copy()
    up_[:y_shp - shift, :] = img_time[shift:, :]
    # Image shifted one step up and one step left
    up_left = img_time.copy()
    up_left[:y_shp - shift, :x_shp - 1] = img_time[shift:, 1:]
    # Image shifted one step up and one step right
    up_right = img_time.copy()
    up_right[:y_shp - shift, 1:] = img_time[shift:, :-1]

    # Merge
    if shift % 2 == 1:
        func = np.logical_not
    else:
        func = np.logical_and
    res = (func(img_time, up_) +
           func(img_time, up_left) +
           func(img_time, up_right))

    return res


def flash_moved_down(img_time, shift):
    """Get subset for flash: shift one row up"""
    y_shp, x_shp = img_time.shape

    # Image shifted up
    down = img_time.copy()
    down[shift:, :] = img_time[:y_shp - shift, :]
    # Image shifted one step down and one step left
    down_left = img_time.copy()
    down_left[shift:, :x_shp - 1] = img_time[:y_shp - shift, 1:]
    # Image shifted one step down and one step right
    down_right = img_time.copy()
    down_right[shift:, 1:] = img_time[:y_shp - shift:, :-1]

    # Merge
    if shift % 2 == 1:
        func = np.logical_not
    else:
        func = np.logical_and
    res = (func(img_time, down) +
           func(img_time, down_left) +
           func(img_time, down_right))

    return res
