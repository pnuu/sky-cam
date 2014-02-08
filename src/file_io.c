
#include "file_io.h"

static int writeImage(char* filename, struct frame img_in, char* title);
static int save_data_to_png(char *filename, struct data_image *data_in, 
			    char *title);
static void write_log(struct stacks *stacks);
static char *make_time_string(struct stacks *stacks, int type);
static void save_ave_stack(struct stacks *stacks);
static void make_fname(char *fname, struct stacks *stacks, int type);
static int file_exists(char *fname);

static int writeImage(char* filename, struct frame img_in, char* title){
  int code = 0, i, j, k;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr = NULL;
  png_bytep row = NULL;
  int n = img_in.width * img_in.height;

  unsigned char *img_out = NULL;
  img_out = (unsigned char*)malloc(n * img_in.channels*
				   sizeof(unsigned char));

  // Open file for writing (binary mode)
  fp = fopen(filename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s for writing\n", filename);
    code = 1;
    goto finalise;
  }
  
  // Initialize write structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf(stderr, "Could not allocate write struct\n");
    code = 1;
    goto finalise;
  }

  // Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fprintf(stderr, "Could not allocate info struct\n");
    code = 1;
    goto finalise;
  }
  
  // Setup Exception handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "Error during png creation\n");
    code = 1;
    goto finalise;
  }
  
  png_init_io(png_ptr, fp);

  unsigned char *Y = img_in.Y;
  if(img_in.channels == 3){
    unsigned char *Cr = img_in.Cr;
    unsigned char *Cb = img_in.Cb;
    /* Calculate color data */
    for(i=0, j=0, k=0; i<n; i+=2, j+=6, k++){
      img_out[j+0] = limit(Y[i] + 1.4075*(Cr[k] - 128));
      img_out[j+1] = limit(Y[i] + 0.3455*(Cb[k] - 128) - 
			   (0.7169*(Cr[k] - 128)));
      img_out[j+2] = limit(Y[i] + 1.7790*(Cb[k] - 128));
      img_out[j+3] = limit(Y[i+1] + 1.4075*(Cr[k] - 128));
      img_out[j+4] = limit(Y[i+1] + 0.3455*(Cb[k] - 128) - 
			   (0.7169*(Cr[k] - 128)));
      img_out[j+5] = limit(Y[i+1] + 1.7790*(Cb[k] - 128));
    }
  } else { /* B&W image */
    /* Copy luminance */
    for(i=0; i<n; i++){
      img_out[i] = Y[i];
    }
  }

  switch(img_in.channels){
  case 3:
    /* Band-interleaved RGB image */
    png_set_IHDR(png_ptr, info_ptr, img_in.width, img_in.height,
		 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  default:
    // B&W image
    // Write header (8 bit gray)
    png_set_IHDR(png_ptr, info_ptr, img_in.width, img_in.height,
		 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  }
  
  // Set title
  if (title != NULL) {
    png_text title_text;
    title_text.compression = PNG_TEXT_COMPRESSION_NONE;
    title_text.key = "Title";
    title_text.text = title;
    png_set_text(png_ptr, info_ptr, &title_text, 1);
  }
  
  png_write_info(png_ptr, info_ptr);
  
  // Allocate memory for one row (1 byte per pixel)
  row = (png_bytep) malloc(img_in.channels *
			   img_in.width *
			   sizeof(png_byte));
  
  // Write image data
  int x, y;
  for (y=0; y<img_in.height; y++) {
    /* Form row pointers */
    for (x=0; x<img_in.channels*img_in.width; x++) {
      row[x] = (img_out[y*img_in.channels*img_in.width + x]);
    }
    png_write_row(png_ptr, row);
  }
  
  // End write
  png_write_end(png_ptr, NULL);

 finalise:
  if(fp != NULL) fclose(fp);
  if(info_ptr != NULL){ png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1); }
  if(info_ptr != NULL){ free(info_ptr); info_ptr = NULL; }
  if(png_ptr != NULL){ png_destroy_write_struct(&png_ptr, (png_infopp)NULL); }
  if(png_ptr != NULL){ free(png_ptr); png_ptr = NULL; }
  if(row != NULL){ free(row); }
  if(img_out != NULL){ free(img_out); }

  return code;
}

static int save_data_to_png(char *filename, struct data_image *data_in, char *title){

  int code = 0, i, j;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr = NULL;
  png_bytep row = NULL;

  int n = data_in->width * data_in->height;

  unsigned char *img_out = NULL;
  img_out = (unsigned char*)malloc(3*n*sizeof(unsigned char));

  // Open file for writing (binary mode)
  fp = fopen(filename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s for writing\n", filename);
    code = 1;
    goto finalise;
  }
  
  // Initialize write structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf(stderr, "Could not allocate write struct\n");
    code = 1;
    goto finalise;
  }

  // Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fprintf(stderr, "Could not allocate info struct\n");
    code = 1;
    goto finalise;
  }
  
  // Setup Exception handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "Error during png creation\n");
    code = 1;
    goto finalise;
  }
  
  png_init_io(png_ptr, fp);
  
  /* Calculate "color" data */
  /* Lowest 8 bits -> blue channel,
     next 8 bits -> green channel,
     next 8 bits -> red channel
  */
  unsigned int temp;
  for(i=0, j=0; i<n; i++, j+=3){
    temp = data_in->Y[i];
    /* red */
    img_out[j+0] = (unsigned char)((temp >> 16) & 0xFF); /* bits 23 - 16*/
    /* green */
    img_out[j+1] = (unsigned char)((temp >> 8) & 0xFF); /* bits 15 - 8 */
    /* blue */
    img_out[j+2] = (unsigned char)(temp & 0xFF); /* bits 7 - 0 */
  }

  /* Band-interleaved RGB image */
  png_set_IHDR(png_ptr, info_ptr, data_in->width, data_in->height,
	       8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Set title
  if (title != NULL) {
    png_text title_text;
    title_text.compression = PNG_TEXT_COMPRESSION_NONE;
    title_text.key = "Title";
    title_text.text = title;
    png_set_text(png_ptr, info_ptr, &title_text, 1);
  }
  
  png_write_info(png_ptr, info_ptr);
  
  // Allocate memory for one row (1 byte per pixel)
  row = (png_bytep) malloc(3 * data_in->width * sizeof(png_byte));
  
  // Write image data
  int x, y;
  for (y=0; y<data_in->height; y++) {
    /* Form row pointers */
    for (x=0; x<3*data_in->width; x++) {
      row[x] = (img_out[y*3*data_in->width + x]);
    }
    png_write_row(png_ptr, row);
  }
  
  // End write
  png_write_end(png_ptr, NULL);

 finalise:
  if(fp != NULL) fclose(fp);
  if(info_ptr != NULL){ png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1); }
  if(info_ptr != NULL){ free(info_ptr); info_ptr = NULL; }
  if(png_ptr != NULL){ png_destroy_write_struct(&png_ptr, (png_infopp)NULL); }
  if(png_ptr != NULL){ free(png_ptr); png_ptr = NULL; }
  if(row != NULL){ free(row); }
  if(img_out != NULL){ free(img_out); }


  return code;

}


void *save_stacks(void *ptr){
  struct stacks *stacks = (struct stacks*)ptr;
  int i = 0, n = stacks->width * stacks->height;

  if(stacks->save_logs > 0){
    write_log(stacks);
  }

  char *fname = (char*)malloc(FNAME_SIZE*sizeof(char));
  memset(fname, '\0', FNAME_SIZE);  

  if(stacks->latest != NULL){
    printf("Saving %s\n", stacks->latest_fname);
    writeImage(stacks->latest_fname, *(stacks->latest), "Finsprite latest stack");  
    for(i=n; i--;){stacks->latest->Y[i] = 0;}
    /*
    if(stacks->channels == 3){
      for(i=n/2; i--;){stacks->latest->Cr[i] = stacks->latest->Cb[i] = 0;}
    }
    */
  }
  
  if(stacks->ave24 != NULL){
    make_fname(fname, stacks, AVE24);
    stacks->ave24->Y[0] = stacks->num_imgs;
    printf("Saving %s, %d frames used\n", fname, stacks->num_imgs);
    save_data_to_png(fname, stacks->ave24, "Sky-cam 24-bit sum");
    
    for(i=n; i--;){stacks->ave24->Y[i] = 0;}
    if(stacks->ave24->Cr != NULL){
      for(i=n/2; i--;){stacks->ave24->Cr[i] = stacks->ave24->Cb[i] = 0;}
    }
  }

  if(stacks->ave8 != NULL){
    save_ave_stack(stacks);
    for(i=n; i--;){stacks->ave8->Y[i] = 0;}
    if(stacks->ave8->Cr != NULL){
      for(i=n/2; i--;){stacks->ave8->Cr[i] = stacks->ave8->Cb[i] = 0;}
    }
  }

  
  if(stacks->max != NULL){
    memset(fname,'\0',FNAME_SIZE);
    make_fname(fname, stacks, PEAK_HOLD);
    printf("Saving %s, %d frames used.\n", fname, stacks->num_imgs);
    //stretch(peak_hold->img_buffer, peak_hold->width*peak_hold->height, 0.01, 0.99);
    writeImage(fname, *(stacks->max), "Finsprite peak-hold stack");

    for(i=n; i--;){stacks->max->Y[i] = 0;}
    /*
    if(stacks->channels == 3){
      for(i=n/2; i--;){stacks->max->Cr[i] = stacks->max->Cb[i] = 0;}
    }
    */
  }

  if(stacks->times != NULL){
    unsigned long time_temp;
    memset(fname,'\0',FNAME_SIZE);
    make_fname(fname, stacks, PIXEL_TIMES);
    printf("Saving %s.\n",fname);
    // Record time to pixels 0, 1 and 2
    stacks->times->Y[0] = stacks->timestamp >> 16;
    stacks->times->Y[1] = stacks->timestamp - (stacks->times->Y[0] << 16);
    stacks->times->Y[2] = (unsigned int)stacks->usec/1000;
    for(i=n; i>2; i--){
      if(stacks->last_times->Y[i] != 0){
        time_temp = (stacks->last_times->Y[i] + stacks->times->Y[i]) >> 1;
        stacks->times->Y[i] = time_temp;
      }
    }
	    
    save_data_to_png(fname, stacks->times, "Pixel times in ms");

    for(i=n; i--;){
      stacks->times->Y[i] = 0;
      stacks->last_times->Y[i] = 0;
    }
  }
  
  if(stacks->min != NULL){
    memset(fname,'\0',FNAME_SIZE);
    make_fname(fname, stacks, MIN_HOLD);
    printf("Saving %s, %d frames used.\n", fname, stacks->num_imgs);
    //stretch(min_hold->img_buffer, min_hold->width*min_hold->height, 0.01, 0.99);
    /* Save image  */
    writeImage(fname, *(stacks->min), "Finsprite minimum stack");

    for(i=n; i--;){stacks->min->Y[i] = 255;}
    /*
    if(stacks->channels == 3){
      for(i=n/2; i--;){stacks->min->Cr[i] = stacks->min->Cb[i] = 255;}
    }
    */
  }
  
  stacks->num_imgs = STACKS_CLEARED;

  free(fname);

  pthread_exit(NULL);

}

static void write_log(struct stacks *stacks){

  /* Save stack information to log */
  char stack_log_fname[FNAME_SIZE] = {'\0'};
  char *tmp = NULL;
  int saved_stacks[6] = {0};

  FILE *log_fid;

  if(stacks->latest != NULL){ saved_stacks[0] = 1; }
  if(stacks->min != NULL){ saved_stacks[1] = 1; }
  if(stacks->max != NULL){ saved_stacks[2] = 1; }
  if(stacks->ave8 != NULL){ saved_stacks[3] = 1; }
  if(stacks->ave24 != NULL){ saved_stacks[4] = 1; }
  if(stacks->times != NULL){ saved_stacks[5] = 1; }

  make_fname(stack_log_fname, stacks, STACK_LOG_NAME);

  /* New file to be opened */
  if(file_exists(stack_log_fname)){
    log_fid = fopen(stack_log_fname,"a");
  } else {
    log_fid = fopen(stack_log_fname,"a");
    fprintf(log_fid,"# start year, start month, start day, "
	    "start hour, start minute, start second, "
	    "end year, end month, end day, end hour, end minute, "
	    "end second, stack length, images in stack, "
	    "latest stack saved, min stack saved, max stack saved, "
	    "8-bit ave stack saved, "
	    "24-bit ave stack saved, pixel times saved\n");
  }
  
  tmp = make_time_string(stacks, STACK_LOG_DATA);
  fprintf(log_fid, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n", 
	  tmp,
	  stacks->length,
	  stacks->num_imgs, 
	  saved_stacks[0],
	  saved_stacks[1],
	  saved_stacks[2],
	  saved_stacks[3],
	  saved_stacks[4],
	  saved_stacks[5]
	  );
  fclose(log_fid);

  free(tmp);
}


static char *make_time_string(struct stacks* stacks, int type){

  char *time_string = (char*)malloc(100*sizeof(char));
  char *tmp = NULL;

  time_t timestamp = stacks->timestamp;
  time_t end_timestamp = stacks->end_timestamp;
  long usec = stacks->usec;
  long end_usec = stacks->end_usec;

  if(type < STACK_LOG_NAME){
    strftime(time_string,30,"%Y-%m-%d_%H%M%S",gmtime(&(timestamp)));
  }

  switch(type){
  case PIXEL_TIMES:
    tmp = (char*)malloc(50*sizeof(char));
    sprintf(tmp,".%.3ld",usec/1000); // time in ms
    strcat(time_string,tmp);
    free(tmp);
    break;
  case STACK_LOG_NAME:
    strftime(time_string,30,"%Y-%m-%d",gmtime(&(timestamp)));
    break;
  case STACK_LOG_DATA:
    tmp = (char*)malloc(30*sizeof(char));
    strftime(time_string,30,"%Y,%m,%d,%H,%M,%S",gmtime(&(timestamp)));
    sprintf(tmp,".%.3ld",usec/1000);
    strcat(time_string,tmp);
    if(type == STACK_LOG_DATA){
      memset(tmp,'\0',30);
      strftime(tmp, 30, ",%Y,%m,%d,%H,%M,%S",
	       gmtime(&(end_timestamp)));
      strcat(time_string, tmp);
      memset(tmp, '\0', 30);
      sprintf(tmp, ".%.3ld", end_usec/1000);
      strcat(time_string, tmp);
    }
    free(tmp);
    break;
  }
  return time_string;
}


static void save_ave_stack(struct stacks *stacks){

  int i, j, num_img = stacks->num_imgs;
  char *fname = (char*)malloc(FNAME_SIZE*sizeof(char));
  memset(fname,'\0',FNAME_SIZE);

  struct frame out_frame;
  out_frame.Y = NULL;
  out_frame.Cr = NULL;
  out_frame.Cb = NULL;

  unsigned int *Y = stacks->ave8->Y;
  unsigned int *Cr = stacks->ave8->Cr;
  unsigned int *Cb = stacks->ave8->Cb;
  int width = stacks->width;
  int height = stacks->height;
  int channels = stacks->channels;

  int n = width*height;

  out_frame.Y = calloc(width *height, sizeof(unsigned char));
  if(channels == 3){
    out_frame.Cr = (unsigned char*)malloc(width * height * 
					  sizeof(unsigned char) / 2);
    out_frame.Cb = (unsigned char*)malloc(width * height * 
					  sizeof(unsigned char) / 2);
  }
  out_frame.channels = channels;
  out_frame.width = width;
  out_frame.height = height;
  //  out_frame.timestamp = stacks->timestamp;
  //  out_frame.usec = stacks->usec;

  if(channels == 3){
    j = 0;
    for(i=0; i<n; i++){
      out_frame.Y[i] = Y[i]/num_img;
      if(i%2 == 0){
	out_frame.Cr[j] = Cr[j]/num_img;
	out_frame.Cb[j] = Cb[j]/num_img;
	j++;
      }
    }
  } else {
    for(i=0; i<n; i++){
      out_frame.Y[i] = Y[i]/num_img;
    }
    //out_frame[i] = 255*ave_stack_save->img_buffer[i]/ave_stack_save->max_val;
    //out_frame[i] = 255*(ave_stack_save->img_buffer[i]-ave_stack_save->min_val)/(ave_stack_save->max_val-ave_stack_save->min_val);
  }

  make_fname(fname, stacks, AVE8);
  //stretch(out_frame, ave_stack_save->width*ave_stack_save->height, 0.02, 0.98);
  printf("Saving %s, %d frames used.\n", fname, num_img);
  writeImage(fname, out_frame, "Finsprite average stack");
  
  free(out_frame.Y);
  if(channels == 3){
    free(out_frame.Cr);
    free(out_frame.Cb);
  }
  free(fname);

}


static void make_fname(char *fname, struct stacks *stacks, int type){
  char tmp[50] = {'\0'};
  mode_t dir_mode = S_IRWXU |S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

  time_t timestamp = stacks->timestamp;
  char *out_dir = stacks->out_dir;
  int stack_length = stacks->length;
  int create_sub_dirs = stacks->create_sub_dirs;

  if(stacks->out_dir != NULL){
    strcat(fname, out_dir);
    strcat(fname,"/");
  }

  if(create_sub_dirs > 0){
    strftime(tmp, 50, "%Y",gmtime(&(timestamp)));
    strcat(fname, tmp);
    memset(tmp,'\0',50);

    if(mkdir(fname, dir_mode) < 0 && errno != EEXIST){
      printf("Unable to create %s\nSaving to current direcory: %d\n",fname,errno);
      // Clear path
      memset(fname, '\0', FNAME_SIZE);
    } else {

      strftime(tmp, 50, "/%m",gmtime(&(timestamp)));
      strcat(fname, tmp); CLEAR(tmp);
      mkdir(fname, dir_mode);

      if(type < STACK_LOG_NAME){
	strftime(tmp,50,"/%d",gmtime(&(timestamp)));
	strcat(fname,tmp); CLEAR(tmp);
	mkdir(fname, dir_mode);
      }

      /* Save stacks to their own dirs
       */
      switch(type){
      case MIN_HOLD:
	strcat(fname,"/min");
	break;
      case PEAK_HOLD:
	strcat(fname,"/max");
	break;
      case PIXEL_TIMES:
	strcat(fname,"/pixel_times");
	break;
      case AVE8:
      case AVE24:
	strcat(fname,"/ave");
	break;
      }

      mkdir(fname, dir_mode);

      memset(tmp, '\0', 50);

      strcat(fname,"/");
    
    }

  }

  if(stacks->prefix != NULL){
    strcat(fname, stacks->prefix);
    strcat(fname, "_");
  }

  char *tmp2 = NULL;
  tmp2 = make_time_string(stacks, type);
  strcat(fname, tmp2); 
  free(tmp2);

  switch(type){
  case AVE8:
    sprintf(tmp,"_UTC_%d_s_ave8.png", stack_length);
    break;
  case AVE24:
    sprintf(tmp,"_UTC_%d_s_ave24.png", stack_length);
    break;
  case PEAK_HOLD:
    sprintf(tmp,"_UTC_%d_s_max.png", stack_length);
    break;
  case PIXEL_TIMES:
    sprintf(tmp,"_UTC_pixel_times.png");
    break;
  case MIN_HOLD:
    sprintf(tmp,"_UTC_%d_s_min.png", stack_length);
    break;
  case STACK_LOG_NAME:
    sprintf(tmp,"_stacks.log");
    break;
  }
  strcat(fname,tmp);

}

static int file_exists(char *fname){

  struct stat st;

  stat(fname, &st);

  if(errno == ENOENT){
    return 0;
  }

  return 1;

}
