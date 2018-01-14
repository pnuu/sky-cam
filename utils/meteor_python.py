import numpy as np


def expand_in_time(meteors, times):
    """Expand detected meteors in time."""
    labels = np.unique(meteors)
    shp = meteors.shape
    for lbl in labels[1:]:
        while True:
            y_idxs, x_idxs = np.where(meteors == lbl)
            min_t = np.min(times[y_idxs, x_idxs])
            max_t = np.max(times[y_idxs, x_idxs])
            num = y_idxs.size

            for y_i, x_i in zip(y_idxs, x_idxs):
                tim = times[y_i, x_i]
                if tim == min_t or tim == max_t:
                    for y_n in range(max(0, y_i - 3), min(shp[0] - 1, y_i + 4)):
                        for x_n in range(max(0, x_i - 3), min(shp[1], x_i + 4)):
                            if np.abs(times[y_n, x_n] - tim) < 0.1:
                                meteors[y_n, x_n] = lbl

            if (meteors == lbl).sum() == num:
                break
