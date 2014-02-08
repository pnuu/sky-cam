
#include "init.h"

void pre_pre_init_stacks(struct stacks *stacks){
  stacks->timestamp = 0;
  stacks->usec = 0;
  stacks->end_timestamp = 0;
  stacks->end_usec = 0;
  stacks->num_imgs = 0;
  stacks->width = 0;
  stacks->height = 0;
  stacks->channels = 1;
  stacks->length = STACK_LENGTH;
  stacks->saturation_limit = SATURATION_LIMIT;
  stacks->create_sub_dirs = 1;
  stacks->save_logs = 1;

  stacks->ave8 = NULL;
  stacks->ave24 = NULL;
  stacks->max = NULL;
  stacks->min = NULL;
  stacks->times = NULL;
  stacks->last_times = NULL;
  stacks->latest = NULL;

  stacks->out_dir = NULL;
  stacks->prefix = NULL;
  stacks->latest_fname = NULL;

}

void pre_init_stacks(struct stacks *stacks){

  if(stacks->ave8 != NULL){
    stacks->ave8->Y = NULL;
    stacks->ave8->Cr = NULL;
    stacks->ave8->Cb = NULL;
  }

  if(stacks->ave24 != NULL){
    stacks->ave24->Y = NULL;
    stacks->ave24->Cr = NULL;
    stacks->ave24->Cb = NULL;
  }

  if(stacks->max != NULL){
    stacks->max->Y = NULL;
    stacks->max->Cr = NULL;
    stacks->max->Cb = NULL;
  }

  if(stacks->min != NULL){
    stacks->min->Y = NULL;
    stacks->min->Cr = NULL;
    stacks->min->Cb = NULL;
  }

  if(stacks->latest != NULL){
    stacks->latest->Y = NULL;
    stacks->latest->Cr = NULL;
    stacks->latest->Cb = NULL;
  }

  if(stacks->times != NULL){
    stacks->times->Y = NULL;
    stacks->last_times->Y = NULL;
  }

}

void init_stacks(struct stacks *stacks, int width, int height, int channels){
  
  stacks->width = width;
  stacks->height = height;

  if(stacks->latest != NULL && stacks->latest->Y == NULL){
    init_latest_img(stacks->latest, width, height, channels);
  }
  if(stacks->times != NULL && stacks->times->Y == NULL){
    stacks->times->Y = calloc(width * height, sizeof(unsigned int));
    stacks->last_times->Y = calloc(width * height, sizeof(unsigned int));
    stacks->times->width = width;
    stacks->times->height = height;
    stacks->last_times->width = width;
    stacks->last_times->height = height;
  }
  if(stacks->max != NULL && stacks->max->Y == NULL){
    init_peak_hold(stacks->max, width, height, channels);
  }
  if(stacks->min != NULL && stacks->min->Y == NULL){
    init_min_hold(stacks->min, width, height, channels);
  }
  if(stacks->ave8 != NULL && stacks->ave8->Y == NULL){
    init_ave_stack(stacks->ave8, width, height, channels);
  }
  if(stacks->ave24 != NULL && stacks->ave24->Y == NULL){
    init_ave_stack(stacks->ave24, width, height, 1);
  }

}

void uninit_stacks(struct stacks *stacks){

  if(stacks->latest != NULL){
    free(stacks->latest->Y);
    if(stacks->channels == 3){
      free(stacks->latest->Cb);
      free(stacks->latest->Cr);
    }
    free(stacks->latest);
  }

  if(stacks->max != NULL){
    free(stacks->max->Y);
    if(stacks->channels == 3){
      free(stacks->max->Cb);
      free(stacks->max->Cr);
    }
    free(stacks->max);
  }

  if(stacks->min != NULL){
    free(stacks->min->Y);
    if(stacks->channels == 3){
      free(stacks->min->Cb);
      free(stacks->min->Cr);
    }
    free(stacks->min);
  }

  if(stacks->ave8 != NULL){
    free(stacks->ave8->Y);
    if(stacks->ave8->Cb != NULL){
      free(stacks->ave8->Cb);
      free(stacks->ave8->Cr);
    }
    free(stacks->ave8);
  }

  if(stacks->ave24 != NULL){
    free(stacks->ave24->Y);
    if(stacks->ave24->Cb != NULL){
      free(stacks->ave24->Cb);
      free(stacks->ave24->Cr);
    }
    free(stacks->ave24);
  }

  if(stacks->times != NULL){
    free(stacks->times->Y);
    free(stacks->last_times->Y);
    if(stacks->times->Cb != NULL){
      free(stacks->times->Cb);
      free(stacks->times->Cr);
      free(stacks->last_times->Cb);
      free(stacks->last_times->Cr);
    }
    free(stacks->times);
    free(stacks->last_times);
  }

}

void init_current_frame(struct frame *current_frame, 
			int width, int height, int channels){
  current_frame->channels = channels;
  current_frame->width = width;
  current_frame->height = height;
  current_frame->Y = (unsigned char*)malloc(width * height *
					    sizeof(unsigned char));
  current_frame->Cr = NULL;
  current_frame->Cb = NULL;
  current_frame->timestamp = 0;
  current_frame->usec = 0;
  if(channels == 3){
    current_frame->Cr = (unsigned char*)malloc(0.5 * width * height *
					       sizeof(unsigned char));
    current_frame->Cb = (unsigned char*)malloc(0.5* width * height *
					       sizeof(unsigned char));
  }

}

void init_peak_hold(struct frame *peak_hold, 
		    int width, int height, int channels){
  peak_hold->Y = calloc(width * height,
			sizeof(unsigned char));
  if(channels == 3){
    peak_hold->Cr = calloc(0.5 * width * height,
			   sizeof(unsigned char));
    peak_hold->Cb = calloc(0.5 * width * height,
			   sizeof(unsigned char));
  }
  peak_hold->channels = channels;
  peak_hold->width = width;
  peak_hold->height = height;
}

void init_min_hold(struct frame *min_hold, 
		   int width, int height, int channels){
  int i = 0;
  min_hold->Y = malloc(width * height*
		       sizeof(unsigned char));
  for(i=0; i<width*height; i++){
    min_hold->Y[i] = 255;
  }
  if(channels == 3){
    min_hold->Cr = malloc(0.5 * width * height *
			  sizeof(unsigned char));
    min_hold->Cb = malloc(0.5 * width * height *
			  sizeof(unsigned char));
  }
  min_hold->channels = channels;
  min_hold->width = width;
  min_hold->height = height;
}

void init_latest_img(struct frame *latest_img, 
		     int width, int height, int channels){
  latest_img->Y = calloc(width * height,
			 sizeof(unsigned char));
  if(channels == 3){
    latest_img->Cr = calloc(0.5 * width * height,
			    sizeof(unsigned char));
    latest_img->Cb = calloc(0.5 * width * height,
			    sizeof(unsigned char));
  }
  latest_img->channels = channels;
  latest_img->width = width;
  latest_img->height = height;
}

void init_ave_stack(struct data_image *ave_stack, 
		    int width, int height, int channels){
  ave_stack->Y = calloc(width * height,
			sizeof(unsigned long));
  if(channels == 3){
    ave_stack->Cr = calloc(0.5 * width * height,
			   sizeof(unsigned long));
    ave_stack->Cb = calloc(0.5 * width * height,
			   sizeof(unsigned long));
  }
  ave_stack->channels = channels;
  ave_stack->width = width;
  ave_stack->height = height;
}

/**************************************************************************
 * Video device and memory map initilisation functions
 *************************************************************************/

void start_capturing(const int fd){
  unsigned int i;
  enum v4l2_buf_type type;
  
  for(i = 0; i < N_MMAP_BUFFERS; ++i) {
    struct v4l2_buffer buf;
    
    CLEAR(buf);
    
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    
    if(xioctl(fd, VIDIOC_QBUF, &buf) == -1)
      errno_exit("VIDIOC_QBUF");
  }
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if(xioctl(fd, VIDIOC_STREAMON, &type) == -1){
    errno_exit("VIDIOC_STREAMON");
  }
}


void stop_capturing(const int fd){
  enum v4l2_buf_type type;
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if(xioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    errno_exit("VIDIOC_STREAMOFF");

}


void init_mmap(const int fd, struct buffer *mmap_buffers){

  struct v4l2_requestbuffers req;
  //struct buffer *mmap_buffers;
  int i = 0;
  
  CLEAR(req);
  
  req.count = N_MMAP_BUFFERS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  
  if(xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    if(EINVAL == errno) {
      fprintf(stderr, "Device does not support memory mapping\n");
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_REQBUFS");
    }
  }
  
  if(req.count < N_MMAP_BUFFERS) {
    fprintf(stderr, "Insufficient buffer memory on device.\n");
    exit(EXIT_FAILURE);
  }
  
  /**  
  if(!mmap_buffers) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }
  */

  for(i = 0; i < req.count; ++i) {

    struct v4l2_buffer buf;    
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    
    if(xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1){
      errno_exit("VIDIOC_QUERYBUF");
    }

    mmap_buffers[i].length = buf.length;
    mmap_buffers[i].start =
      mmap(NULL /* start anywhere */,
	   buf.length,
	   PROT_READ | PROT_WRITE /* required */,
	   MAP_SHARED /* recommended */,
	   fd, buf.m.offset);
    
    if(MAP_FAILED == mmap_buffers[i].start)
      errno_exit("mmap");
  }

}

void uninit_mmap(struct buffer *mmap_buffers){
  unsigned int i;
  
  for(i = 0; i < N_MMAP_BUFFERS; ++i){
    if(munmap(mmap_buffers[i].start, mmap_buffers[i].length) == -1)
      errno_exit("munmap");
  }
}


void init_device(const int fd, const char *dev_name, 
			struct v4l2_format *fmt){
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  unsigned int min;
  
  if(xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
    if(EINVAL == errno) {
      fprintf(stderr, "%s is no V4L2 device\n", dev_name);
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_QUERYCAP");
    }
  }
  
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n",
	    dev_name);
    exit(EXIT_FAILURE);
  }
  
  if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf(stderr, "%s does not support streaming i/o\n",
	    dev_name);
    exit(EXIT_FAILURE);
  }
  
  /* Select video input, video standard and tune here. */
  
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if(xioctl(fd, VIDIOC_CROPCAP, &cropcap) == 0) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */
    
    if(xioctl(fd, VIDIOC_S_CROP, &crop) == -1) {
      switch(errno) {
      case EINVAL:
	/* Cropping not supported. */
	break;
      default:
	/* Errors ignored. */
	break;
      }
    }
  } else {
    /* Errors ignored. */
  }
    
  CLEAR(*fmt);
  
  fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt->fmt.pix.width = 720;
  fmt->fmt.pix.height = 576;
  fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;

  if(xioctl(fd, VIDIOC_S_FMT, fmt) == -1)
    errno_exit("VIDIOC_S_FMT");
  
  /* Note VIDIOC_S_FMT may change width and height. */
  
  /* Buggy driver paranoia. */
  min = fmt->fmt.pix.width * 2;
  if(fmt->fmt.pix.bytesperline < min)
    fmt->fmt.pix.bytesperline = min;
  min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
  if(fmt->fmt.pix.sizeimage < min)
    fmt->fmt.pix.sizeimage = min;

}


int open_device(const char *dev_name){
  struct stat st; 
  int fd;
  
  if(stat(dev_name, &st) == -1) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	    dev_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if(!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", dev_name);
    exit(EXIT_FAILURE);
  }
  
  fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
  
  if(fd == -1) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
	    dev_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  return fd;

}


int close_device(int fd){
  if(close(fd) == -1)
    errno_exit("close");
  
  return -1;
}

void errno_exit(const char *s){
  fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

int xioctl(int fd, int request, void *arg){
  int r;
  
  do {
    r = ioctl(fd, request, arg);
  } while(r == -1 && EINTR == errno);
  
  return r;
}
