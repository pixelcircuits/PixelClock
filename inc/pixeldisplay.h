//-----------------------------------------------------------------------------------------
// Title:	Pixel Display (raspbian)
// Program: Pixel Display Driver
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef PIXEL_DISPLAY_H
#define PIXEL_DISPLAY_H


#define PIXEL_DISPLAY_CLOSE_ACTION_OFF 0
#define PIXEL_DISPLAY_CLOSE_ACTION_RESET 1
#define PIXEL_DISPLAY_CLOSE_ACTION_STANDBY 2

//! Initializes the pixel display.
void pixelDisplay_init();

//! Closes the pixel display into the given mode.
void pixelDisplay_close(unsigned char action);

//! Clears the pixel display buffer with the given color.
void pixelDisplay_clear(unsigned char red, unsigned char green, unsigned char blue);

//! Writes a color value to the display buffer at the given position.
void pixelDisplay_write(unsigned int x, unsigned int y, unsigned char red, unsigned char green, unsigned char blue);

//! Reads a color value to the display buffer at the given position.
void pixelDisplay_read(unsigned int x, unsigned int y, unsigned char *red, unsigned char *green, unsigned char *blue);

//! Sets the brightness value for the display [0-3].
void pixelDisplay_setBrightness(unsigned char brightness);

//! Gets the brightness value for the display [0-3].
unsigned char pixelDisplay_getBrightness();

//! Sends display buffer data and brightness settings to the display.
void pixelDisplay_flush();

#endif
