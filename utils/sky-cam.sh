#!/bin/bash

#hours=1
#mins=0
#secs=5

TH=100
N=15
LATEST_LENGTH=60
#LATEST_OUT=/home/pnuu/Dropbox/Public/sky-cam/latest.png
LATEST_OUT=/home/pnuu/kuvat/finsprite/latest.png
#LENGTH=`echo "$hours * 3600 + $mins * 60 + $secs" | bc`
BIN=/home/pnuu/bin/sky-cam
OUT=/home/pnuu/kuvat/finsprite
DEV=/dev/easycap0
MASK=mask3.png
STACKS="Me"
PREFIX=pnuu
enable_trigger=0

cd $OUT
t=`ps -A | grep 'sky-cam.sh' | wc -l`
echo $t
if [ $t -le 2 ]; then
  if [ $enable_trigger -gt 0 ]; then
    $BIN -c $DEV -$STACKS -p $PREFIX -n $N -t -T $TH -k $MASK -O $LATEST_OUT -L $LATEST_LENGTH
  else
    $BIN -c $DEV -$STACKS -p $PREFIX -O $LATEST_OUT -L $LATEST_LENGTH
  fi
fi


