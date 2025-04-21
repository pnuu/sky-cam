/*
 *    Video stacking program for Video 4 Linux 2 devices.
 *
 *    Panu Lahtinen, pnuu+skycam@iki.fi
 */

#include "sky-cam.h"

int main(int argc, char **argv) {
    char *dev_name = "/dev/video0";

    struct stacks stacks1, stacks2;
    unsigned int delay_start = 60;
    int delay_between_frames = -1;
    struct v4l2_format fmt;
    unsigned long capture_length = ULONG_MAX;

    int fd;

    pre_pre_init_stacks(&stacks1);
    pre_pre_init_stacks(&stacks2);

    for (;;) {
        int index;
        int c;

        c = getopt_long(argc, argv, short_options, long_options, &index);

        if (c == -1) break;

        switch (c) {
            case 0: /* getopt_long() flag */
                break;
            case 'c':
                dev_name = optarg;
                break;
            case 'h':
                usage(stdout, argc, argv);
                exit(EXIT_SUCCESS);
            case 's': /* length of stacked period in secs */
                sscanf(optarg, "%d", &(stacks1.length));
                stacks2.length = stacks1.length;
                break;
            case 'm': /* Enable minimum stack */
                stacks1.min = malloc(sizeof(struct frame));
                stacks2.min = malloc(sizeof(struct frame));
                break;
            case 'M': /* Enable maximum stack */
                stacks1.max = malloc(sizeof(struct frame));
                stacks2.max = malloc(sizeof(struct frame));
                break;
            case 'x': /* Enable pixel time image */
                stacks1.times = malloc(sizeof(struct data_image));
                stacks2.times = malloc(sizeof(struct data_image));
                stacks1.last_times = malloc(sizeof(struct data_image));
                stacks2.last_times = malloc(sizeof(struct data_image));
                break;
            case 'a': /* Enable 8-bit average stack */
                stacks1.ave8 = malloc(sizeof(struct data_image));
                stacks2.ave8 = malloc(sizeof(struct data_image));
                break;
            case 'A': /* Enable 24-bit average stack */
                stacks1.ave24 = malloc(sizeof(struct data_image));
                stacks2.ave24 = malloc(sizeof(struct data_image));
                break;
            case 'e': /* Enable saving of "latest.png" */
                stacks1.latest = malloc(sizeof(struct frame));
                stacks2.latest = malloc(sizeof(struct frame));
                break;
            case 'd':
                sscanf(optarg, "%d", &delay_between_frames);
                break;
            case 'D':
                sscanf(optarg, "%ud", &delay_start);
                break;
            case 'l': /* Length of imaging period */
                sscanf(optarg, "%ld", &capture_length);
                break;
            case 'p': /* Image prefix */
                stacks1.prefix = optarg;
                stacks2.prefix = optarg;
                break;
            case 'o': /* Output directory prefix */
                stacks1.out_dir = optarg;
                stacks2.out_dir = optarg;
                break;
            case 'O': /* Filename for "latest.png" */
                stacks1.latest_fname = optarg;
                stacks2.latest_fname = optarg;
                break;
            case 'S': /* Set saturation pixel value */
                sscanf(optarg, "%d", &(stacks1.saturation_limit));
                stacks2.saturation_limit = stacks1.saturation_limit;
                break;
            case 'C':
                sscanf(optarg, "%d", &(stacks1.channels));
                stacks2.channels = stacks1.channels;
                break;
            case 'N':
                stacks1.create_sub_dirs = stacks2.create_sub_dirs = -1;
                break;
            case 'L':
                stacks1.save_logs = stacks2.save_logs = -1;
                break;
            case 'v':
                printf("Version: %d\n", VERSION);
                exit(EXIT_SUCCESS);
            /*
            case 'f':
                sscanf(optarg,"%d",&video_input_fps);
                break;
            case 'V': // Verbose
                verbose = 1;
                break;
            */
            default:
                usage(stderr, argc, argv);
                exit(EXIT_FAILURE);
        }
    }

    if (stacks1.min == NULL && stacks1.max == NULL && stacks1.ave8 == NULL &&
        stacks1.ave24 == NULL && stacks1.latest == NULL) {
        printf("\nNothing to do, terminating.\n");
        printf("Specify at least one imaging mode!\n\n");
        usage(stderr, argc, argv);
        exit(EXIT_FAILURE);
    }

    if (stacks1.channels != 1 && stacks1.channels != 3) {
        stacks1.channels = 1;
        stacks2.channels = 1;
    }

    if (stacks1.latest != NULL && stacks1.latest_fname == NULL) {
        stacks1.latest_fname = (char *)malloc(FNAME_SIZE * sizeof(char));
        stacks2.latest_fname = (char *)malloc(FNAME_SIZE * sizeof(char));
        strcpy(stacks1.latest_fname, "latest.png");
        strcpy(stacks2.latest_fname, "latest.png");
    }

    pre_init_stacks(&stacks1);
    pre_init_stacks(&stacks2);

    fd = open_device(dev_name);

    init_device(fd, dev_name, &fmt);

    mainloop(fd, delay_start, delay_between_frames, capture_length, &stacks1,
             &stacks2, fmt);

    fd = close_device(fd);

    uninit_stacks(&stacks1);
    uninit_stacks(&stacks2);

    exit(EXIT_SUCCESS);

    return 0;
}

static void usage(FILE *fp, int argc, char **argv) {
    fprintf(
        fp,
        "Usage: %s [options]\n\n"
        "GENERAL OPTIONS\n"
        "-h | --help    Print this message\n"
        "-c | --capture-device <name> Video device name [/dev/video0]\n"
        "-l | --capture-length <num>    Length of imaging period in\n"
        "                                                         seconds "
        "[default: unlimited]\n"
        "-d | --delay-between-frames <num>    Delay <num> ms between reading "
        "frames\n"
        "-D | --delay-start <num>    Delay start of imaging for <num> frames. "
        "Useful\n"
        "                                                    eg. for cameras "
        "requiring time to adjust for\n"
        "                                                    brightness or "
        "white "
        "balance [default: 10]\n"
        "-p | --prefix <prefix>    Image filename prefix [default: none]\n"
        "-o | --out-dir <dir>    Image output directory [default: current "
        "directory]\n"
        "                                            The images are saved to "
        "<dir>/yyyy/mm/dd/\n"
        "-O | --latest-fname <file>    Filename with full path for saving\n"
        "                                                        "
        "\"latest.png\" "
        "[default: latest.png]\n"
        "-s | --stack-length <num>    Length of stacked period in seconds "
        "[60]\n"
        "-C | --number_of_channels <num>    Number of color channels (1 or 3) "
        "[default: 1]\n"
        "-S | --saturation-limit <num>    Set pixel saturation value for time "
        "image\n"
        "                                                             "
        "calculation [default: 255]\n"
        "-N | --no-sub-dirs    Do not create daily sub-directories\n"
        "-L | --no-logs    Do not save log files\n"
        "\nSTACKS\n"
        "-m | --min    Enable minimum stacking [default: off]\n"
        "-M | --max    Enable maximum stacking [default: off]\n"
        "-x | --pixel-times    Enable pixel time image [default: off]\n"
        "-a | --ave8    Enable 8-bit average stack [default: off]\n"
        "-A | --ave24    Enable 24-bit average stack [default: off]\n"
        "-e | --latest    Save \"latest.png\", an independent peak-hold "
        "stack\n"
        //        "-f | --fps <num>    FPS requested from the video device
        //        [default: auto]\n"
        "\n",
        argv[0]);
}

static void mainloop(int fd, int delay_start, int delay_between_frames,
                     unsigned long capture_length, struct stacks *stacks1,
                     struct stacks *stacks2, struct v4l2_format fmt) {
    time_t timenow;
    time_t start_time;
    struct frame cur_frame;
    struct frame *current_frame = &cur_frame;
    struct stacks *stacks = stacks1;
    struct stacks *stacks_save = stacks2;
    static pthread_t stack_save_thread;
    static pthread_attr_t thread_attr_detached;
    struct buffer *mmap_buffers;
    int status = 0;

    init_current_frame(current_frame, fmt.fmt.pix.width, fmt.fmt.pix.height,
                       stacks1->channels);

    init_stacks(stacks, fmt.fmt.pix.width, fmt.fmt.pix.height,
                stacks1->channels);
    init_stacks(stacks_save, fmt.fmt.pix.width, fmt.fmt.pix.height,
                stacks1->channels);

    mmap_buffers = calloc(N_MMAP_BUFFERS, sizeof(*mmap_buffers));
    init_mmap(fd, mmap_buffers);
    start_capturing(fd);

    pthread_attr_init(&thread_attr_detached);
    pthread_attr_setdetachstate(&thread_attr_detached, PTHREAD_CREATE_DETACHED);

    start_time = time(NULL);
    timenow = time(NULL);

    while (difftime(timenow, start_time) < capture_length) {
        if (delay_between_frames > 0) {
            usleep(1000 * delay_between_frames);
            delay_start = 0;
        }
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (r == -1) {
                if (EINTR == errno) continue;

                errno_exit("select");
            }

            if (r == 0) {
                fprintf(stderr, "select timeout\n");
                exit(EXIT_FAILURE);
            }

            if (read_frame(fd, mmap_buffers, current_frame, fmt)) break;
        }

        if (delay_start > 0) {
            delay_start--;
        } else {
            if (stacks->timestamp == 0) {
                stacks->timestamp = current_frame->timestamp;
                stacks->usec = current_frame->usec;
                stacks->num_imgs = 0;
            }
            stacks->end_timestamp = current_frame->timestamp;
            stacks->end_usec = current_frame->usec;
            stacks->num_imgs++;

            process_stack(current_frame, stacks);

            if (stacks->length <
                difftime(current_frame->timestamp, stacks->timestamp)) {
                /* Swap stack pointers and create thread for stack
                 * saving */
                if (stacks == stacks1) {
                    stacks = stacks2;
                    stacks_save = stacks1;
                } else {
                    stacks = stacks1;
                    stacks_save = stacks2;
                }

                stacks->timestamp = 0;
                stacks->usec = 0;

                void *arg = stacks_save;

                status =
                    pthread_create(&stack_save_thread, &thread_attr_detached,
                                   save_stacks, arg);
                if (status) {
                    printf(
                        "Thread \"save_stacks()\" failed "
                        "with code %d\n",
                        status);
                }
            }
        }
        timenow = time(NULL);
    }

    /* Free thread attributes */
    pthread_attr_destroy(&thread_attr_detached);
    stop_capturing(fd);
    uninit_mmap(mmap_buffers);
    free(mmap_buffers);
    mmap_buffers = NULL;

    if (cur_frame.Y != NULL) {
        free(cur_frame.Y);
        if (cur_frame.channels == 3) {
            free(cur_frame.Cb);
            free(cur_frame.Cr);
        }
    }
}

static int read_frame(int fd, struct buffer *mmap_buffers,
                      struct frame *current_frame, struct v4l2_format fmt) {
    struct v4l2_buffer buf;
    time_t frametime;
    struct timespec t_spec;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        switch (errno) {
            case EAGAIN:
                printf("EAGAIN\n");
                return 0;

            case EIO:
                /* Could ignore EIO, see spec. */
                printf("EIO\n");
                /* fall through */

            default:
                printf("default: %d\n", errno);
                errno_exit("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < N_MMAP_BUFFERS);

    clock_gettime(CLOCK_REALTIME, &t_spec);
    frametime = buf.timestamp.tv_sec;

    if (frametime > 0) {
        current_frame->timestamp = t_spec.tv_sec;
        current_frame->usec = t_spec.tv_nsec / 1000;
        frametime = time(NULL);

        process_frame(mmap_buffers[buf.index].start, current_frame, fmt);

        if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            errno_exit("VIDIOC_QBUF");
        }

        return 1;
    }

    return -1;
}

static void process_frame_grey(unsigned char *p, unsigned char *cur_Y,
                               unsigned long sizeimage) {
    int i, j;
    for (i = 0, j = 0; i < sizeimage; i += 4, j += 2) {
        cur_Y[j + 0] = p[j];
        cur_Y[j + 1] = p[j + 2];
    }
}

static void process_frame_yuv_single_chan(unsigned char *p,
                                          unsigned char *cur_Y,
                                          unsigned long sizeimage,
                                          int offsets[]) {
    int i, j;
    for (i = 0, j = 0; i < sizeimage; i += 4, j += 2) {
        cur_Y[j + 0] = p[i + offsets[0]];
        cur_Y[j + 1] = p[i + offsets[1]];
    }
}

static void process_frame_uyvy_single_chan(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned long sizeimage) {
    int offsets[] = {1, 3};
    process_frame_yuv_single_chan(p, cur_Y, sizeimage, offsets);
}

static void process_frame_yuyv_single_chan(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned long sizeimage) {
    int offsets[] = {0, 2};
    process_frame_yuv_single_chan(p, cur_Y, sizeimage, offsets);
}

static void process_frame_yuv_three_chans(
    unsigned char *p, unsigned char *cur_Y, unsigned char *cur_Cb,
    unsigned char *cur_Cr, unsigned long sizeimage, int offsets[]) {
    int i, j, k;
    for (i = 0, j = 0, k = 0; i < sizeimage; i += 4, j += 2, k++) {
        cur_Y[j + 0] = p[i + offsets[0]];
        cur_Y[j + 1] = p[i + offsets[1]];
        cur_Cb[k] = p[i + offsets[2]];
        cur_Cr[k] = p[i + offsets[3]];
    }
}

static void process_frame_uyvy_three_chans(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned char *cur_Cb,
                                           unsigned char *cur_Cr,
                                           unsigned long sizeimage) {
    int offsets[] = {1, 3, 0, 2};
    process_frame_yuv_three_chans(p, cur_Y, cur_Cb, cur_Cr, sizeimage, offsets);
}

static void process_frame_yuyv_three_chans(unsigned char *p,
                                           unsigned char *cur_Y,
                                           unsigned char *cur_Cb,
                                           unsigned char *cur_Cr,
                                           unsigned long sizeimage) {
    int offsets[] = {0, 2, 1, 3};
    process_frame_yuv_three_chans(p, cur_Y, cur_Cb, cur_Cr, sizeimage, offsets);
}

static int process_frame(unsigned char *p, struct frame *current_frame,
                         struct v4l2_format fmt) {
    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int width = current_frame->width, height = current_frame->height;
    int channels = current_frame->channels;
    unsigned long sizeimage = fmt.fmt.pix.sizeimage;
    unsigned long pixelformat = fmt.fmt.pix.pixelformat;

    if (cur_Y == NULL) {
        cur_Y = malloc(width * height * sizeof(unsigned char));
        if (channels == 3) {
            cur_Cr = malloc(width * height * sizeof(unsigned char) / 2);
            cur_Cb = malloc(width * height * sizeof(unsigned char) / 2);
        }
    }

    /* Assume YUYV format. RGB and GREY should be added? */
    switch (pixelformat) {
        case V4L2_PIX_FMT_GREY:
            process_frame_grey(p, cur_Y, sizeimage);
            break;
        case V4L2_PIX_FMT_UYVY:
            if (channels == 1) {
                process_frame_uyvy_single_chan(p, cur_Y, sizeimage);
            } else {  // number_of_channels == 3
                process_frame_uyvy_three_chans(p, cur_Y, cur_Cb, cur_Cr,
                                               sizeimage);
            }
            break;
        case V4L2_PIX_FMT_YUYV:
            if (channels == 1) {
                process_frame_yuyv_single_chan(p, cur_Y, sizeimage);
            } else {  // number_of_channels == 3
                process_frame_yuyv_three_chans(p, cur_Y, cur_Cb, cur_Cr,
                                               sizeimage);
            }

        default:
            break;
    }
    if (cur_Y[1000] > 0) {
        return 1;
    } else {
        return -1;
    }
}

static void stack_max_latest_time(struct frame *current_frame,
                                  struct stacks *stacks) {
    int i = 0, j = 0, k = 1;
    unsigned int ms_since_frame_start = 0;

    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;
    time_t cur_timestamp = current_frame->timestamp;
    long cur_usec = current_frame->usec;

    time_t stack_timestamp = stacks->timestamp;
    long stack_usec = stacks->usec;

    int saturation_limit = stacks->saturation_limit;

    unsigned char *M_Y = stacks->max->Y;
    unsigned char *l_Y = stacks->latest->Y;
    unsigned int *t_Y = stacks->times->Y;
    unsigned int *tl_Y = stacks->last_times->Y;
    ms_since_frame_start = 1000 * difftime(cur_timestamp, stack_timestamp) +
                           0.001 * (cur_usec - stack_usec);

    unsigned char *M_Cr = stacks->max->Cr;
    unsigned char *M_Cb = stacks->max->Cb;
    unsigned char *l_Cr = stacks->latest->Cr;
    unsigned char *l_Cb = stacks->latest->Cb;

    if (channels == 3) {
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = l_Y[i] = cur_Y[i];
                t_Y[i] = ms_since_frame_start;
            }
            if (cur_Y[i] >= saturation_limit) {
                tl_Y[i] = ms_since_frame_start;
            }
            if (cur_Y[k] > M_Y[k]) {
                M_Y[k] = l_Y[k] = cur_Y[k];
                t_Y[k] = ms_since_frame_start;
                M_Cr[j] = l_Cr[j] = cur_Cr[j];
                M_Cb[j] = l_Cb[j] = cur_Cb[j];
            }
            if (cur_Y[k] >= saturation_limit) {
                tl_Y[k] = ms_since_frame_start;
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] > M_Y[i]) {
                t_Y[i] = ms_since_frame_start;
                M_Y[i] = l_Y[i] = cur_Y[i];
            }
            if (cur_Y[i] >= saturation_limit) {
                tl_Y[i] = ms_since_frame_start;
            }
        }
    }
}

static void stack_max_latest(struct frame *current_frame,
                             struct stacks *stacks) {
    int i = 0, j = 0, k = 1;

    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;

    unsigned char *M_Y = stacks->max->Y;
    unsigned char *l_Y = stacks->latest->Y;

    if (channels == 3) {
        unsigned char *M_Cr = stacks->max->Cr;
        unsigned char *M_Cb = stacks->max->Cb;
        unsigned char *l_Cr = stacks->max->Cr;
        unsigned char *l_Cb = stacks->max->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = l_Y[i] = cur_Y[i];
                M_Cr[j] = l_Cr[j] = cur_Cr[j];
                M_Cb[j] = l_Cb[j] = cur_Cb[j];
            }
            if (cur_Y[k] > M_Y[k]) {
                M_Y[k] = l_Y[k] = cur_Y[k];
                M_Cr[j] = l_Cr[j] = cur_Cr[j];
                M_Cb[j] = l_Cb[j] = cur_Cb[j];
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = l_Y[i] = cur_Y[i];
            }
        }
    }
}

static void stack_max_time(struct frame *current_frame, struct stacks *stacks) {
    int i = 0, j = 0, k = 1;
    unsigned int ms_since_frame_start = 0;
    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;
    time_t cur_timestamp = current_frame->timestamp;
    long cur_usec = current_frame->usec;

    time_t stack_timestamp = stacks->timestamp;
    long stack_usec = stacks->usec;

    int saturation_limit = stacks->saturation_limit;

    unsigned char *M_Y = stacks->max->Y;
    unsigned int *t_Y = stacks->times->Y;
    unsigned int *tl_Y = stacks->last_times->Y;
    ms_since_frame_start = 1000 * difftime(cur_timestamp, stack_timestamp) +
                           0.001 * (cur_usec - stack_usec);
    if (channels == 3) {
        unsigned char *M_Cr = stacks->max->Cr;
        unsigned char *M_Cb = stacks->max->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = cur_Y[i];
                t_Y[i] = ms_since_frame_start;
            }
            if (cur_Y[i] >= saturation_limit) {
                tl_Y[i] = ms_since_frame_start;
            }
            if (cur_Y[k] > M_Y[k]) {
                M_Y[k] = cur_Y[k];
                t_Y[k] = ms_since_frame_start;
                M_Cr[j] = cur_Cr[j];
                M_Cb[j] = cur_Cb[j];
            }
            if (cur_Y[k] >= saturation_limit) {
                tl_Y[k] = ms_since_frame_start;
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = cur_Y[i];
                t_Y[i] = ms_since_frame_start;
            }
            if (cur_Y[i] >= saturation_limit) {
                tl_Y[i] = ms_since_frame_start;
            }
        }
    }
}

static void stack_max(struct frame *current_frame, struct stacks *stacks) {
    int i = 0, j = 0, k = 1;
    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;

    unsigned char *M_Y = stacks->max->Y;

    if (channels == 3) {
        unsigned char *M_Cr = stacks->max->Cr;
        unsigned char *M_Cb = stacks->max->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = cur_Y[i];
            }
            if (cur_Y[k] > M_Y[k]) {
                M_Y[k] = cur_Y[k];
                M_Cr[j] = cur_Cr[j];
                M_Cb[j] = cur_Cb[j];
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] > M_Y[i]) {
                M_Y[i] = cur_Y[i];
            }
        }
    }
}

void static stack_latest(struct frame *current_frame, struct stacks *stacks) {
    int i = 0, j = 0, k = 1;

    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;

    unsigned char *l_Y = stacks->latest->Y;

    if (channels == 3) {
        unsigned char *l_Cr = stacks->latest->Cr;
        unsigned char *l_Cb = stacks->latest->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] > l_Y[i]) {
                l_Y[i] = cur_Y[i];
            }
            if (cur_Y[k] > l_Y[k]) {
                l_Y[k] = cur_Y[k];
                l_Cr[j] = cur_Cr[j];
                l_Cb[j] = cur_Cb[j];
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] > l_Y[i]) {
                l_Y[i] = cur_Y[i];
            }
        }
    }
}

void static stack_min(struct frame *current_frame, struct stacks *stacks) {
    int i = 0, j = 0, k = 1;
    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;
    unsigned char *m_Y = stacks->min->Y;
    if (channels == 3) {
        unsigned char *m_Cr = stacks->min->Cr;
        unsigned char *m_Cb = stacks->min->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            if (cur_Y[i] < m_Y[i]) {
                m_Y[i] = cur_Y[i];
            }
            if (cur_Y[k] < m_Y[k]) {
                m_Y[k] = cur_Y[k];
                m_Cr[j] = cur_Cr[j];
                m_Cb[j] = cur_Cb[j];
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (cur_Y[i] < m_Y[i]) {
                m_Y[i] = cur_Y[i];
            }
        }
    }
}

void static stack_ave8(struct frame *current_frame, struct stacks *stacks) {
    int i = 0, j = 0, k = 1;
    unsigned char *cur_Y = current_frame->Y;
    unsigned char *cur_Cb = current_frame->Cb;
    unsigned char *cur_Cr = current_frame->Cr;
    int n = current_frame->width * current_frame->height;
    int channels = current_frame->channels;

    unsigned int *a_Y = stacks->ave8->Y;
    if (channels == 3) {
        unsigned int *a_Cr = stacks->ave8->Cr;
        unsigned int *a_Cb = stacks->ave8->Cb;
        for (i = 0, j = 0, k = 1; i < n; i += 2, j++, k += 2) {
            a_Y[i] += cur_Y[i];
            a_Y[k] += cur_Y[k];
            a_Cr[j] += cur_Cr[j];
            a_Cb[j] += cur_Cb[j];
        }
    } else {
        for (i = 0; i < n; i++) {
            a_Y[i] += cur_Y[i];
        }
    }
}

void static stack_ave24(struct frame *current_frame, struct stacks *stacks) {
    int i = 0;
    unsigned char *cur_Y = current_frame->Y;
    int n = current_frame->width * current_frame->height;
    unsigned int *A_Y = stacks->ave24->Y;
    for (i = 0; i < n; i++) {
        A_Y[i] += cur_Y[i];
    }
}

static void process_stack(struct frame *current_frame, struct stacks *stacks) {
    time_t cur_timestamp = current_frame->timestamp;
    long cur_usec = current_frame->usec;

    stacks->end_timestamp = cur_timestamp;
    stacks->end_usec = cur_usec;

    if (stacks->num_imgs == STACKS_CLEARED) {
        stacks->timestamp = cur_timestamp;
        stacks->usec = cur_usec;
        stacks->num_imgs = 0;
    }

    if (stacks->max != NULL && stacks->latest != NULL) {
        if (stacks->times != NULL) {
            stack_max_latest_time(current_frame, stacks);
        } else {
            stack_max_latest(current_frame, stacks);
        }
    } else if (stacks->max != NULL) {
        if (stacks->times != NULL) {
            stack_max_time(current_frame, stacks);
        } else {
            stack_max(current_frame, stacks);
        }
    } else if (stacks->latest != NULL) {
        stack_latest(current_frame, stacks);
    }

    if (stacks->min != NULL) {
        stack_min(current_frame, stacks);
    }

    if (stacks->ave8 != NULL) {
        stack_ave8(current_frame, stacks);
    }

    if (stacks->ave24 != NULL) {
        stack_ave24(current_frame, stacks);
    }
}
