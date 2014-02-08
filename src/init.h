
#include "defs.h"

#ifndef INIT_H
#define INIT_H

void pre_pre_init_stacks(struct stacks *stacks);

void pre_init_stacks(struct stacks *stacks);

void init_stacks(struct stacks *stacks, 
		 int width, int height, int channels);

void uninit_stacks(struct stacks *stacks);

void init_current_frame(struct frame *frame, 
			int width, int height, int channels);

void init_peak_hold(struct frame *peak_hold,
		    int width, int height, int channels);

void init_min_hold(struct frame *min_hold,
		   int width, int height, int channels);

void init_latest_img(struct frame *latest_img, 
		     int width, int height, int channels);

void init_ave_stack(struct data_image *ave_stack, 
		    int width, int height, int channels);

void start_capturing(const int fd);

void stop_capturing(const int fd);
void init_mmap(const int fd, struct buffer *mmap_buffers);
void uninit_mmap(struct buffer *mmap_buffers);
void init_device(const int fd, const char *dev_name, 
		 struct v4l2_format *fmt);
int open_device(const char *dev_name);
int close_device(int fd);
void errno_exit(const char *s);
int xioctl(int fd, int request, void *arg);

#endif
