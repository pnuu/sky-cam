
Sky-Cam is a command-line video stacking software for linux. The software is designed to generate stacked images from a video device.

=== Prerequisities ===

First of all, you'll need a computer and a video device supported by V4L2 interface. The video device can be a webcam or a security camera (Watec, etc.) connected to a supported video capture card. At the moment, only devices with YUYV output (monochrome or color) are supported. Support for devices with direct RGB output might be available some day in the future.

Secondly, you'll need a linux distribution providing a build environment (gcc and make) and the needed libraries (libpng). If the libraries are not available, they can be downloaded for free and compiled.

Secondly, you'll need the Sky-Cam source code and a build environment.


=== Install required software and libraries ===

To install the Sky-Cam software, start by installing the prerequisite libraries and compilation tools:

In Debian, Ubuntu, etc., issue the command below in a terminal window (or install via graphical package manager):

sudo apt-get install gcc make libpng12-dev

The installation might suggest some other packages, install also those.


=== Compilation ===

Extract the software source code:

unzip sky-cam_YYYYMMDD.zip

where YYYYMMDD is the version number (date).

Compile the software:

cd sky-cam_YYYYMMDD
make

After this, there should be an executable called sky-cam within the source directory. You can copy the executable to a more suitable place, eg. to bin directory in your home directory.


=== Features ===

To see the list of command-line parameters, issue the following command in terminal within the directory where the software is located:

./sky-cam -h

Usage: ./sky-cam [options]

GENERAL OPTIONS
-h | --help  Print this message
-c | --capture-device <name> Video device name [/dev/video0]
-l | --capture-length <num>  Length of imaging period in
                             seconds [default: unlimited]
-d | --delay-between-frames <num>  Delay <num> ms between reading frames
-D | --delay-start <num>  Delay start of imaging for <num> frames. Useful
                          eg. for cameras requiring time to adjust for
                          brightness or white balance [default: 10]
-p | --prefix <prefix>  Image filename prefix [default: none]
-o | --out-dir <dir>  Image output directory [default: current directory]
                      The images are saved to <dir>/yyyy/mm/dd/
-O | --latest-fname <file>  Filename with full path for saving
                            "latest.png" [default: latest.png]
-s | --stack-length <num>  Length of stacked period in seconds [60]
-C | --number_of_channels <num>  Number of color channels (1 or 3) [default: 1]
-S | --saturation-limit <num>  Set pixel saturation value for time image
                               calculation [default: 255]
-N | --no-sub-dirs  Do not create daily sub-directories
-L | --no-logs  Do not save log files

STACKS
-m | --min  Enable minimum stacking [default: off]
-M | --max  Enable maximum stacking [default: off]
-x | --pixel-times  Enable pixel time image [default: off]
-a | --ave8  Enable 8-bit average stack [default: off]
-A | --ave24  Enable 24-bit average stack [default: off]
-e | --latest  Save "latest.png", an independent peak-hold stack


The most basic usage is to make stacks with the brightest value of each pixel from a 60 second long period. These stacks will be saved every minute, until the software is stopped. The images are saved to daily directories (yyyy/mm/dd/max/yyyy-mm-dd_hhmmss_max.png).

sky-cam -M

This assumes, that the video device is /dev/video0. If not, use commandline switch -c to give the correct device location:

sky-cam -M -c /dev/easycap0

If the default stack length of 60 seconds is not ideal, the stack length can be defined as follows (2 minutes given as 120 seconds):

sky-cam -M -c /dev/video0 -s 120

These commands will save the images under the directory where the command was issued. To define the save location, use -o:

sky-cam -M -o /existing/directory/somewhere

It is also possible to save a maximum stack (brightest value of each pixel) with a fixed filename of "latest.png" (eg. for use in a web-page). The imaging period for this can be defined separately with -L (default: 60 seconds). If the default filename "latest.png" is not suitable, define a new one, with full path, using -O.

sky-cam -e -L 30 -O /existing/directory/somewhere/other_filename.png

Several different stacks can be saved at the same time. All of the stacks will have the same imaging period. See definition of the stacks from the listing at the beginning of this chapter.

sky-cam -meMaA -c /dev/video0

Some cameras require some time to adjust the exposure and/or white balance, so by default 60 first frames after the start of the program will be discarded. This value can be changed with -D.

In some uses, there's no point in continuing the imaging forever. To limit the time the stacks are collected, the capture length can be set with -l (here, 5 hours, or 5*60*60 = 18000 seconds):

sky-cam -M -l 18000


=== Continuous "operational" use ===

Two template scripts are provided with the source code, namely "sky-cam.sh" and "sky-cam_day.sh". These scripts can be used to control separately which stacks are saved at different time. For example, for TLEs and meteors, the interesting stuff happens at night, and these images are saved. Even if the day time images are not interesting in the sense of observation of these phenomena, the software can be used to save weather camera data to be used in web page with a consant filename, and the disks won't be filled with images.

