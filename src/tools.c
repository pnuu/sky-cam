#include "tools.h"

void histogram_eq(unsigned char *in, int n) {
    int cdf_in[256] = {0};
    int cdf_out[256] = {0};
    int i, j;
    unsigned char cdf_min = 255;

    // generate the input CDF (cumulative distribution function)
    for (i = n; i--;) {
        for (j = in[i]; j < 256; j++) {
            cdf_in[j] += 1;
        }
    }
    // find the minimum of CDF
    for (i = 256; i--;) {
        if (cdf_in[i] < cdf_min) {
            cdf_min = cdf_in[i];
        }
    }

    // find the output CDF
    for (i = 256; i--;) {
        cdf_out[i] = round(255 * (cdf_in[i] - cdf_min) / (n - cdf_min));
    }

    // adjust the histogram
    for (i = n; i--;) {
        in[i] = cdf_out[in[i]];
    }
}

void stretch(unsigned char *in, int n, float p_low, float p_up) {
    int i, sum;
    unsigned int hist[256];
    unsigned char lower_val = 0;
    unsigned char higher_val = 0;
    double p5 = n * p_low;
    double p95 = n * p_up;

    unsigned char hist_max = 0, hist_min = 255;

    for (i = 256; i--;) {
        hist[i] = 0;
    }

    for (i = n; i--;) {
        hist[in[i]]++;
    }

    for (i = 256; i--;) {
        if (hist[i] > hist_max) {
            hist_max = i;
        }
    }
    for (i = 256; i--;) {
        if (hist[i] < hist_min) {
            hist_min = i;
        }
    }

    // Check that we don't try to scale constant value image
    if (hist_max != hist_min) {
        sum = 0;
        for (i = 256; i--;) {
            sum += hist[i];
            if (sum < p5) {
                lower_val++;
            }
            if (sum < p95) {
                higher_val++;
            }
        }

        for (i = n; i--;) {
            p5 = 255 * (in[i] - lower_val) / (higher_val - lower_val);
            if (p5 > 255) {
                in[i] = 255;
            } else {
                if (p5 < 0) {
                    in[i] = 0;
                } else {
                    in[i] = p5;
                }
            }
        }
    }
}

unsigned char limit(double val) {
    if (val < 0) {
        return 0;
    }
    if (val > 255) {
        return 255;
    }

    return (unsigned char)round(val);
}
