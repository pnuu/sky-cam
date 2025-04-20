
#include <math.h>

#ifndef TOOLS_H
#define TOOLS_H

void histogram_eq(unsigned char *in, int n);
void stretch(unsigned char *in, int n, float p_low, float p_up);
unsigned char limit(double val);

#endif
