#!/bin/bash
BASEDIR=$(dirname "$0")
cd $BASEDIR

#delete old ClockUI
sudo rm -rf /home/pi/ClockUI
sudo mkdir /home/pi/ClockUI

#copy over new ClockUI
sudo cp -a * /home/pi/ClockUI/

#build
cd /home/pi/ClockUI
sudo apt-get install wiringpi
sudo make clean
sudo make