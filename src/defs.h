#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>  /* getopt_long() */
#include <fcntl.h>  /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <asm/types.h>  /* for videodev2.h */
#include <linux/videodev2.h>
#include <pthread.h>
#include <png.h>

#define VERSION 20131110
#define FNAME_SIZE 255
#define SATURATION_LIMIT 255
#define STACK_LENGTH 60

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define NONE -1
#define IMG 0
#define MIN_HOLD 1
#define PEAK_HOLD 2
#define PIXEL_TIMES 3
#define AVE8 4
#define AVE24 5
#define HIST 6
#define STRETCH 7
#define HISTOGRAM 8
#define DAY_DIR 9
#define STACK_LOG_NAME 10
#define STACK_LOG_DATA 11

#define N_MMAP_BUFFERS 8

#define STACKS_CLEARED -1

/* Struct definitions */

/* Video frame */
struct buffer {
  void *start;
  size_t length;
};

struct frame {
  unsigned char *Y;
  unsigned char *Cb; // half of size(Y)
  unsigned char *Cr; // half of size(Y)
  int width;
  int height;
  int channels;
  time_t timestamp;
  long usec;
};

struct data_image {
  unsigned int *Y;
  unsigned int *Cr; // if used, half of size(Y)
  unsigned int *Cb; // if used, half of size(Y)
  int width;
  int height;
  int channels;
  /*
  time_t timestamp;
  long usec;
  time_t end_timestamp;
  long end_usec;
  int num_imgs;
  unsigned long min_val;
  unsigned long max_val;
  */
};

/* Struct with pointers to stacks */
struct stacks {
  struct frame *latest;
  struct frame *max;
  struct frame *min;
  struct data_image *ave8;
  struct data_image *ave24;
  struct data_image *times;
  struct data_image *last_times;
  time_t timestamp;
  long usec;
  time_t end_timestamp;
  long end_usec;
  int num_imgs;
  int width;
  int height;
  int channels;
  int length;
  int fps;
  int saturation_limit;
  int create_sub_dirs;
  int save_logs;

  char *out_dir;
  char *prefix;
  char *latest_fname;
};

#endif

