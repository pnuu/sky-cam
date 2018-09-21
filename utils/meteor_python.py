import numpy as np


def expand_in_time(x_in, y_in, times):
    """Expand detected meteors in time."""
    meteors = np.zeros(times.shape)
    meteors[y_in, x_in] = 1
    shp = meteors.shape
    i = 0
    while True:
        y_idxs, x_idxs = np.where(meteors == 1)
        min_t = np.min(times[y_idxs, x_idxs])
        max_t = np.max(times[y_idxs, x_idxs])
        num = y_idxs.size

        for y_i, x_i in zip(y_idxs, x_idxs):
            tim = times[y_i, x_i]
            if tim == min_t or tim == max_t:
                for y_n in range(max(0, y_i - 3),
                                 min(shp[0] - 1, y_i + 4)):
                    for x_n in range(max(0, x_i - 3),
                                     min(shp[1] - 1, x_i + 4)):
                        if np.abs(times[y_n, x_n] - tim) < 150:
                            meteors[y_n, x_n] = 1

        if (meteors == 1).sum() == num:
            break
        i += 1
    y_idxs, x_idxs = np.where(meteors == 1)

    return x_idxs, y_idxs
