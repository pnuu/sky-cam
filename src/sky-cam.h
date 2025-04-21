#include "defs.h" /* Things needed in several modules */
#include "file_io.h"
#include "init.h"
// #include "tools.h"

#ifndef SKYCAM_H
#define SKYCAM_H

/* Commandline options */
static const char short_options[] = "c:s:mMxaAed:D:p:o:O:l:C:f:S:vNLh";

/* Long commandline options */
static const struct option long_options[] = {
    {"capture-device", required_argument, NULL, 'c'},
    {"stack-length", required_argument, NULL, 's'},
    {"min", no_argument, NULL, 'm'},
    {"max", no_argument, NULL, 'M'},
    {"pixel-times", no_argument, NULL, 'x'},
    {"ave8", no_argument, NULL, 'a'},
    {"ave24", no_argument, NULL, 'A'},
    {"latest", no_argument, NULL, 'e'},
    {"delay-between-frames", required_argument, NULL, 'd'},
    {"delay-start", required_argument, NULL, 'D'},
    {"prefix", required_argument, NULL, 'p'},
    {"out-dir", required_argument, NULL, 'o'},
    {"latest-fname", required_argument, NULL, 'O'},
    {"capture-length", required_argument, NULL, 'l'},
    {"number-of-channels", required_argument, NULL, 'C'},
    {"fps", required_argument, NULL, 'f'},
    {"verbose", no_argument, NULL, 'V'},
    {"saturation-limit", required_argument, NULL, 'S'},
    {"version", no_argument, NULL, 'v'},
    {"no-sub-dirs", no_argument, NULL, 'N'},
    {"no-logs", no_argument, NULL, 'L'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}};

static void usage(FILE *fp, int argc, char **argv);
static void mainloop(int fd, int delay_start, int delay_between_frames,
                     unsigned long capture_length, struct stacks *stacks1,
                     struct stacks *stacks2, struct v4l2_format fmt);
static int read_frame(int fd, struct buffer *mmap_buffers,
                      struct frame *current_frame, struct v4l2_format fmt);
static void process_frame_grey(unsigned char *p, unsigned char *cur_Y,
                               unsigned long sizeimage);
static void process_frame_yuv_single_chan(unsigned char *p,
                                          unsigned char *cur_Y,
                                          unsigned long sizeimage,
                                          int offsets[]);
static void process_frame_uyvy_single_chan(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned long sizeimage);
static void process_frame_yuyv_single_chan(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned long sizeimage);
static void process_frame_yuv_three_chans(
    unsigned char *p, unsigned char *cur_Y, unsigned char *cur_Cb,
    unsigned char *cur_Cr, unsigned long sizeimage, int offsets[]);
static void process_frame_uyvy_three_chans(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned char *cur_Cb,
                                           unsigned char *cur_Cr,
                                           unsigned long sizeimage);
static void process_frame_yuyv_three_chans(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned char *cur_Cb,
                                           unsigned char *cur_Cr,
                                           unsigned long sizeimage);

static int process_frame(unsigned char *p, struct timeb tmb,
                         struct frame *current_frame, struct v4l2_format fmt);

static void stack_max_latest_time(struct frame *current_frame,
                                  struct stacks *stacks);
static void stack_max_latest(struct frame *current_frame,
                             struct stacks *stacks);
static void stack_max_time(struct frame *current_frame, struct stacks *stacks);
static void stack_max(struct frame *current_frame, struct stacks *stacks);
static void stack_latest(struct frame *current_frame, struct stacks *stacks);
static void stack_min(struct frame *current_frame, struct stacks *stacks);
static void stack_ave8(struct frame *current_frame, struct stacks *stacks);
static void stack_ave24(struct frame *current_frame, struct stacks *stacks);

static void process_stack(struct frame *current_frame, struct stacks *stacks);

#endif
