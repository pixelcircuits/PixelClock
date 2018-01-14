# Pixel Clock
The Pixel Clock is a small program that simply displays the time along with different pixel art and information grabbed from the web. It was written to run on the Raspberry Pi connected to a Pixel Display. The different looks of the clock are organized into folders and referred to as ‘faces’. The faces can be created, deleted or modified according to different preferences.

* [Hackaday](https://hackaday.io/project/28447-pixel-clock) - Project Page
* [Pixel Display](https://www.tindie.com/products/11311/) - Tindie
* [Pixel Display](http://www.pixelcircuits.com/) - Website

## Setup (Raspbian Desktop)
The Pixel Clock has only been tested on the Raspbian OS (2017-09-07) and should work with any version of the Raspberry Pi. The following steps are for the full desktop version of Raspbian. Refer to [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/README.md) first for getting Raspbian OS up and running. For a read-only mode with Raspbian lite (to avoid SD card corruption when pulling the power), refer to the below section for setup instead.

**1. Configure Wifi**

Setup wifi connection by clicking on the wifi icon, selecting your wifi network and entering the password.

**2. Enable SPI**

Under ‘Preferences -> Raspberry Pi Configuration’, check enabled for SPI under ‘Interfaces’ and click ‘OK’.

**3. Download and Build**

Open a command prompt and enter the following commands to download and build the Pixel Clock. Refer to the configuration section for setting up the correct timezone and other basic options.
```
sudo apt-get install wiringpi
git clone https://github.com/pixelcircuits/PixelClock 
cd PixelClock
make
```

**4. Run on Boot**

To run the Pixel Clock simply run the executable build in the project folder. The Pixel Clock can be set to run on boot by opening a command prompt and editing the rc.local file.
```
sudo nano /etc/rc.local
```
add
```
cd /home/pi/PixelClock
sudo ./PixelClock & 
```
and press Ctrl+X then Y, to save

## Setup (Raspbian Lite with Read-Only Mode)

The Pixel Clock works great on even the most lightweight Raspberry Pi setup (Raspberry Pi Zero W with Raspbian Lite). Using Raspbian Lite has the advantage of enabling a read-only mode to avoid possible SD card corruption when simply pulling the plug on the setup when you’re done. Refer to [this tutorial](https://www.raspberrypi.org/documentation/installation/installing-images/README.md) for getting Raspbian OS up and running. 

**1. Configure Wifi**

Refer to [this tutorial](https://www.raspberrypi.org/documentation/configuration/wireless/wireless-cli.md) for a more in depth way of connecting to a wifi network, but the easiest way is to directly edit the config file through the following commands.
```
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
```
add
```
network={
   ssid="testing"
   psk="testingPassword"
}
```
and press Ctrl+X then Y, to save. Then run 
```
wpa_cli -i wlan0 reconfigure 
```
to immediately try connecting.

**2. Enable SPI**

Enable SPI (and optionally SSH for remote connection) by running the raspberry pi configuration 
```
sudo raspi-config 
```
and navigating to ‘Interface Options’.

**3. Prepare Flash Drive**

The recommended way to get Pixel Clock configured and installed on Raspbian Lite is by setting the project up on a flash drive and running the installation script from there. This makes it easy to setup any clock faces on a computer with a desktop and GUI and simply have them copied to the Raspberry Pi from your flash drive. Just download the project as a ZIP, extract it, configure your desired clock faces in the ‘faces’ folder and copy the whole project onto a flash drive. Refer to the configuration section for setting up the correct timezone and other basic options.

**4. Install From Flash Drive**

Plug in the flash drive to the Raspberry Pi and run the following commands to mount the drive and run the installation script, which copies over the entire project and builds it. Note that this process can be repeated later if you you want to update configured clock faces.
```
sudo mkdir /media/USBFlash
sudo mount /dev/sda1 /media/USBFlash
bash /media/USBFlash/<PATH>/PixelClock/Install.sh
```
*'PATH' is the location of the project folder on your flash drive

**5. Run on Boot**

The Pixel Clock can be set to run on boot by opening a command prompt and editing the rc.local file.
```
sudo nano /etc/rc.local
```
add 
```
cd /home/pi/PixelClock
sudo ./PixelClock & 
```
and press Ctrl+X then Y, to save.
    
**6. Read-Only Script (optional)**

Optionally, you can use Adafruit’s read-only mode script to make it safe to pull the plug on the Pixel Clock without the possibility of corrupting the SD card. Just remember to set up the Read/Write mode jumper during the process or you won’t be able to make changes to your configured clock faces later. Checkout Adafruit’s tutorial [here](https://learn.adafruit.com/read-only-raspberry-pi/) or run the following commands to get it started.
```
wget https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/master/read-only-fs.sh
sudo bash read-only-fs.sh
```

## Configuration

The Pixel Clock project folder contains a global configuration file (config.txt) with a few simple configuration options. The first is **systemLocation** which needs to be changed to a path to be appended to https://www.timeanddate.com/worldclock/ to fetch the accurate time on startup. Follow the link, select your location and then copy that path to the config file. The second config option is **noURLCheckCert** which if set to 1 will bypass certification check for fetched URLs. For some reason this is necessary if you ran the read-only mode script, otherwise set it to 0 for added security.

## Configuring Clock Faces
The Pixel Clock was designed to cycle through different designs and layouts referred to as faces. Each clock face is contained within their own folder that provides a background image (background.png or background.bmp) and sprite image (sprite.pngor sprite.bmp), neither of which are required. An optional config file (config.txt) can be placed in the folder to provide additional details about the face’s behavior.

Each image can consist of a transparent color as well as multiple frames for animation. By default, a pink pixel (R:255, G:0, B:255) denotes transparency and light blue vertical lines (R:0, G:255, B:255) denote separate frames of animation. These color keys can be modified in the configuration file. Also, note that for animated images, each frame must be the exact same size.

The configuration file must be a plain text file where each option is on it’s own line and keys are separated from values by a colon. String values must be encased between either a single quote or double quote, and array values must be comma separated between square brackets. Below are a list of all configuration options:

**duration** *(Int)* - The duration of the face in seconds.

**timePos** *(String[‘none’,‘center’,‘topRight’,‘topLeft’,‘bottomLeft’,‘bottomRight’])* - Position of the time text on the face.

**backgroundFile** *(String)* - Filename for the background image.

**spriteFile** *(String)* -  Filename for the sprite image.

**spritePos** *(Int[2])* - The X and Y position of the sprite image (from top left). 

**clearColor** *(Int[3])* - The RGB color to indicate transparency in the images.

**animColor** *(Int[3])* - The RGB color to indicate animation frames in the images.

**animSpeed** *(Int)* - The duration of a single frame in milliseconds.

**dataURL** *(String)* - The URL of the site to fetch data from.

**dataFilterAfter** *(String)* - Text found after the data to extract.

**dataFilterBefore** *(String)* - Text found before the data to extract. 

**dataFilterIndex** *(Int)* - The index of the data in the page, that matches filter specifications, to use.

**dataType** *(String[‘text’,‘number’])* - The render style of the data (‘number’ is monospaced and ‘text’ is more legible)

**dataPos** *(Int[2])* - The X and Y position of the data text.

**dataAlign** *(String[‘left’,’right’])* - The alignment of the data text from it’s origin.

## License

This project is licensed under the GPL License - see the [LICENSE.md](LICENSE.md) file for details
