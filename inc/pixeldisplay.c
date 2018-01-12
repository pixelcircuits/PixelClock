//-----------------------------------------------------------------------------------------
// Title:	Pixel Display (linux)
// Program: Pixel Display Driver
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "pixeldisplay.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define PIXEL_WIDTH		32
#define PIXEL_HEIGHT	16
#define SYNC_BYTE 		0x01 
#define META_RESET		0x20
#define META_STANDBY	0x10
#define	SPI_CHAN		0
#define	SPI_SPEED		2640000
#define SPI_PIN_CLK 	14
#define SPI_PIN_DAT 	12 
#define SPI_DELAY_US	4000
#define BUFFER_SIZE		((PIXEL_WIDTH*PIXEL_HEIGHT*3)+2)
#define MIN_FPS 2

static int pixeldisplay_spiFd = -1;
static unsigned char pixeldisplay_brightness = 3;
static unsigned char *pixeldisplay_spiBufferWorking = 0;
static unsigned char *pixeldisplay_spiBufferFlush = 0;
static unsigned char *pixeldisplay_spiBufferTransfer = 0;
static unsigned char pixeldisplay_copyingFlushBuffer = 0;
static unsigned char pixeldisplay_frameReadyFlag = 0;

static pthread_t pixeldisplay_thread;
static int pixeldisplay_threadRun = 0;
unsigned long pixeldisplay_getTimestamp() {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return ((currentTime.tv_sec-((currentTime.tv_sec/1000)*1000))*1000000) + currentTime.tv_usec;
}
void pixeldisplay_output(unsigned char metadata) {
	if(pixeldisplay_spiBufferTransfer >= 0 && pixeldisplay_spiBufferFlush >= 0 && pixeldisplay_spiFd >= 0) {		
		int j=0;
		pixeldisplay_copyingFlushBuffer = 1;
		for(;j<BUFFER_SIZE;j++) pixeldisplay_spiBufferTransfer[j] = pixeldisplay_spiBufferFlush[j];
		pixeldisplay_copyingFlushBuffer = 0;
		
		pixeldisplay_spiBufferTransfer[BUFFER_SIZE-1] = SYNC_BYTE;
		pixeldisplay_spiBufferTransfer[BUFFER_SIZE-2] = metadata + pixeldisplay_brightness;
		if (wiringPiSPIDataRW(SPI_CHAN, pixeldisplay_spiBufferTransfer, BUFFER_SIZE) == -1)
		{
			printf ("[PixelDisplay] SPI failure: %s\n", strerror(errno));
		}
		usleep(SPI_DELAY_US);
	}
}
void *pixeldisplay_outputLoop(void *args) {
	long maxTime = 1000000/MIN_FPS;
	while(pixeldisplay_threadRun) {		
	
		long startTime = pixeldisplay_getTimestamp();
		while(pixeldisplay_frameReadyFlag == 0 && (pixeldisplay_getTimestamp()-startTime) < maxTime) usleep(1000);
		pixeldisplay_frameReadyFlag = 0;
		
		pixeldisplay_output(0);
	}
	
	pixeldisplay_threadRun = 1;
	pthread_exit(NULL);
}

//! Initializes the pixel display.
void pixelDisplay_init()
{	
	wiringPiSetup();
	if((pixeldisplay_spiBufferWorking = (unsigned char*)malloc(BUFFER_SIZE)) == NULL)
	{
		fprintf(stderr, "[PixelDisplay] Unable to allocate display buffer: %s\n", strerror(errno));
		pixelDisplay_close(0);
	}
	else if((pixeldisplay_spiBufferFlush = (unsigned char*)malloc(BUFFER_SIZE)) == NULL)
	{
		fprintf(stderr, "[PixelDisplay] Unable to allocate display buffer: %s\n", strerror(errno));
		pixelDisplay_close(0);
	}
	else if((pixeldisplay_spiBufferTransfer = (unsigned char*)malloc(BUFFER_SIZE)) == NULL)
	{
		fprintf(stderr, "[PixelDisplay] Unable to allocate display buffer: %s\n", strerror(errno));
		pixelDisplay_close(0);
	}
	else if((pixeldisplay_spiFd = wiringPiSPISetup(SPI_CHAN, SPI_SPEED)) < 0) 
	{
		fprintf(stderr, "[PixelDisplay] Can't open the SPI bus: %s\n", strerror(errno));
		pixelDisplay_close(0);
	}
	else if(pthread_create(&pixeldisplay_thread, NULL, pixeldisplay_outputLoop, 0) < 0)
	{
		fprintf(stderr, "[PixelDisplay] Failed to create output thread: %s\n", strerror(errno));
		pixelDisplay_close(0);
	}
	
	if(pixeldisplay_spiBufferWorking > 0 && pixeldisplay_spiBufferFlush > 0 && pixeldisplay_spiBufferTransfer > 0) {
		pixeldisplay_threadRun = 1;
		fprintf(stdout, "[PixelDisplay] Successfully initialized.\n");
		int i=0;
		for(;i<BUFFER_SIZE;i++) {
			pixeldisplay_spiBufferWorking[i] = 0;
			pixeldisplay_spiBufferFlush[i] = 0;
			pixeldisplay_spiBufferTransfer[i] = 0;
		}
	} else {
		fprintf(stderr, "[PixelDisplay] Failed to initialize.\n");
	}
}

//! Closes the pixel display.
void pixelDisplay_close(unsigned char action)
{
	if(pixeldisplay_threadRun) {
		pixeldisplay_threadRun = 0;
		while(!pixeldisplay_threadRun);
	}
	pixeldisplay_threadRun = 0;
	
	if(action == 1){ 
		pixeldisplay_output(META_RESET); 
		pixeldisplay_output(META_RESET); 
	} else if(action == 2){ 
		pixeldisplay_output(META_STANDBY); 
		pixeldisplay_output(META_STANDBY); 
		pinMode(SPI_PIN_CLK, OUTPUT);
		digitalWrite(SPI_PIN_CLK, HIGH);
	}
	
	if(pixeldisplay_spiBufferWorking > 0) free(pixeldisplay_spiBufferWorking);
	if(pixeldisplay_spiBufferFlush > 0) free(pixeldisplay_spiBufferFlush);
	if(pixeldisplay_spiBufferTransfer > 0) free(pixeldisplay_spiBufferTransfer);
	if(pixeldisplay_spiFd >= 0) close(pixeldisplay_spiFd);
	
	pixeldisplay_spiFd = -1;
	pixeldisplay_spiBufferWorking = 0;
	pixeldisplay_spiBufferFlush = 0;
	pixeldisplay_spiBufferTransfer = 0;
	fprintf(stdout, "[PixelDisplay] Successfully closed.\n");
}

//! Clears the pixel display buffer with the given color (default: black)
void pixelDisplay_clear(unsigned char red, unsigned char green, unsigned char blue)
{
	if(pixeldisplay_spiBufferWorking >= 0) {
		int i=0;
		for(;i<PIXEL_WIDTH*PIXEL_HEIGHT*3;i+=3) {
			pixeldisplay_spiBufferWorking[i+0] = red;
			pixeldisplay_spiBufferWorking[i+1] = green;
			pixeldisplay_spiBufferWorking[i+2] = blue;
		}
	}
}

//! Writes a color value to the display buffer at the given position.
void pixelDisplay_write(unsigned int x, unsigned int y, unsigned char red, unsigned char green, unsigned char blue)
{
	if(pixeldisplay_spiBufferWorking >= 0) {
		if(x < PIXEL_WIDTH && y < PIXEL_HEIGHT) {
			int index = (PIXEL_WIDTH*y + x)*3;
			pixeldisplay_spiBufferWorking[index+0] = red;
			pixeldisplay_spiBufferWorking[index+1] = green;
			pixeldisplay_spiBufferWorking[index+2] = blue;
		}
	}
}

//! Reads a color value to the display buffer at the given position.
void pixelDisplay_read(unsigned int x, unsigned int y, unsigned char *red, unsigned char *green, unsigned char *blue)
{
	if(pixeldisplay_spiBufferWorking >= 0) {
		if(x>=PIXEL_WIDTH) x = PIXEL_WIDTH-1;
		if(y>=PIXEL_HEIGHT) y = PIXEL_HEIGHT-1;
	
		int index = (PIXEL_WIDTH*y + x)*3;
		*red = pixeldisplay_spiBufferWorking[index+0];
		*green = pixeldisplay_spiBufferWorking[index+1];
		*blue = pixeldisplay_spiBufferWorking[index+2];
	}
}

//! Sets the brightness value for the display [0-3].
void pixelDisplay_setBrightness(unsigned char brightness)
{
	if(brightness > 3) brightness = 3;
	pixeldisplay_brightness = brightness;
}

//! Gets the brightness value for the display [0-3].
unsigned char pixelDisplay_getBrightness()
{
	return pixeldisplay_brightness;
}

//! Sends display buffer data and brightness settings to the display.
void pixelDisplay_flush()
{
	while(pixeldisplay_copyingFlushBuffer > 0);
	if(pixeldisplay_spiBufferWorking >= 0 && pixeldisplay_spiBufferFlush >= 0) {
		unsigned char *tmp = pixeldisplay_spiBufferFlush;
		pixeldisplay_spiBufferFlush = pixeldisplay_spiBufferWorking;
		pixeldisplay_spiBufferWorking = tmp;
		
		int i=0;
		for(;i<BUFFER_SIZE;i++) pixeldisplay_spiBufferWorking[i] = pixeldisplay_spiBufferFlush[i];
	}
	pixeldisplay_frameReadyFlag = 1;
}
