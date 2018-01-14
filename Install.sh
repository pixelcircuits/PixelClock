#!/bin/bash
BASEDIR=$(dirname "$0")
cd $BASEDIR

#delete old ClockUI
sudo rm -rf /home/pi/PixelClock
sudo mkdir /home/pi/PixelClock

#copy over new ClockUI
sudo cp -a * /home/pi/PixelClock/

#build
cd /home/pi/PixelClock
sudo apt-get install wiringpi
sudo make clean
sudo make
