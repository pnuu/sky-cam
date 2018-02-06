import numpy as np
cimport numpy as np
cimport cython

ctypedef np.float64_t DTYPEF64_t
ctypedef np.int64_t DTYPEINT_t

cdef inline int int_max(int a, int b): return a if a >= b else b
cdef inline int int_min(int a, int b): return a if a <= b else b
cdef inline double float64_abs(double a): return a if a >= 0 else -a

@cython.boundscheck(False)
@cython.wraparound(False)
@cython.nonecheck(False)
cpdef void expand_in_time(np.ndarray[DTYPEINT_t, ndim=2] meteors,
                           np.ndarray[DTYPEF64_t, ndim=2] times):
    """Expand detected meteors in time."""
    cdef np.ndarray[DTYPEINT_t, ndim=1] labels = np.unique(meteors)
    cdef int lbl
    cdef int num, num2
    cdef int y_i, x_i, y_n, x_n
    cdef int i, j
    cdef int y__, x__
    cdef double tim, min_t, max_t
    cdef list y_idxs
    cdef list x_idxs

    y__ = meteors.shape[0]
    x__ = meteors.shape[1]

    for lbl in labels[1:]:
        while True:
            y_idxs, x_idxs = own_where(meteors, lbl)
            min_t = float64_min(times, y_idxs, x_idxs)
            max_t = float64_max(times, y_idxs, x_idxs)
            num = len(y_idxs)
            for i in range(num):
                y_i = y_idxs[i]
                x_i = x_idxs[i]
                tim = times[y_i, x_i]
                if tim == min_t or tim == max_t:
                    for y_n in range(int_max(0, y_i - 3),
		                     int_min(y__ - 1, y_i + 4)):
                        for x_n in range(int_max(0, x_i - 3),
			                 int_min(x__, x_i + 4)):
                            if float64_abs(times[y_n, x_n] - tim) < 100.:
                                meteors[y_n, x_n] = lbl

            num2 = 0
            for i in range(y__):
                for j in range(x__):
                    if meteors[i, j] == lbl:
                        num2 += 1
            if num == num2:
                break

@cython.boundscheck(False)
@cython.wraparound(False)
@cython.nonecheck(False)
cdef double float64_min(np.ndarray[DTYPEF64_t, ndim=2] times,
                        list y_idxs,
                        list x_idxs):
    cdef int i
    cdef double val = 1e63

    for i in range(len(y_idxs)):
        if val > times[y_idxs[i], x_idxs[i]]:
            val = times[y_idxs[i], x_idxs[i]]

    return val

@cython.boundscheck(False)
@cython.wraparound(False)
@cython.nonecheck(False)
cdef double float64_max(np.ndarray[DTYPEF64_t, ndim=2] times,
                        list y_idxs,
                        list x_idxs):
    cdef int i  # , x, y
    cdef double val = -1e63

    # for i in range(y_idxs.size):
    for i in range(len(y_idxs)):
        if val < times[y_idxs[i], x_idxs[i]]:
            val = times[y_idxs[i], x_idxs[i]]

    return val

@cython.boundscheck(False)
@cython.wraparound(False)
@cython.nonecheck(False)
cdef tuple own_where(np.ndarray[DTYPEINT_t, ndim=2] arr, int lbl):
    cdef int i, j
    cdef list y_idxs = [], x_idxs = []
    cdef int x_size, y_size

    y_size = arr.shape[0]
    x_size = arr.shape[1]
    for i in range(y_size):
        for j in range(x_size):
            if arr[i, j] == lbl:
                y_idxs.append(i)
                x_idxs.append(j)
    return y_idxs, x_idxs
