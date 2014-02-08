#!/bin/bash

BIN=/home/pnuu/bin/sky-cam
OUT=/home/pnuu/kuvat/finsprite/latest.png
DEV=/dev/easycap0
STACKS="e"
PREFIX=pnuu

LATEST_LENGTH=60

t=`ps -A | grep 'sky-cam_day' | wc -l`
if [ $t -le 2 ]; then
  $BIN -c $DEV -$STACKS -L $LATEST_LENGTH -O $OUT
fi

