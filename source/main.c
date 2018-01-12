//-----------------------------------------------------------------------------------------
// Title:	Clock UI
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "render.h"
#include "config.h"
#include "web.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define DATABUFFER_SIZE 1024
#define DATA_SCROLL_DELAY 32

//Global Data
const char* timeURL = "https://www.timeanddate.com/worldclock/";
signed int secondsDelta;

//Util Functions
void syncTime(char* location, char noCert);
int getTimeInSeconds();
void renderTime(char position, char renderColon);
void renderData(const char* text, unsigned int textWidth, unsigned int time, signed int* position, unsigned char dataAlign, unsigned char dataType);

//! Main Loop
int main(void)
{
	render_init();
	render_setBrightness(3);
	
	//data
	char databuffer[DATABUFFER_SIZE];
	char fullpath[CONFIG_MAX_FILENAME_SIZE*4];
	char foldername[CONFIG_MAX_FILENAME_SIZE];
	int i,j,t=6;
	
	//system configuration
	systemConfig sys = config_getSystemConfig();
	syncTime(sys.systemLocation, sys.noURLCheckCert);
	
	//main loop
	while(t) {
		config_getNextFolder(foldername);
		config c = config_getConfig(foldername);
	
		unsigned int textWidth=0;
		databuffer[0] = 0;
		sprite back = new_sprite(config_constructFilePath(fullpath, foldername, c.backgroundFile), 0, 0);
		sprite sprte = new_sprite(config_constructFilePath(fullpath, foldername, c.spriteFile), c.clearColor, c.animColor);
		if(c.dataURL[0]) {
			web_request(databuffer, DATABUFFER_SIZE, c.dataURL, c.dataFilterAfter, c.dataFilterBefore, c.dataFilterIndex, sys.noURLCheckCert);
			if(c.dataType == CONFIG_DATA_TYPE_TEXT) textWidth = render_getTextWidth(databuffer, FONT_SIZE_LARGE);
			if(c.dataType == CONFIG_DATA_TYPE_NUMBER) {
				for(i=0, j=0; i<DATABUFFER_SIZE; i++) if(databuffer[i]==0 || (databuffer[i]<=57 && databuffer[i]>=48)) databuffer[j++] = databuffer[i];
				textWidth = render_getTextWidth(databuffer, FONT_SIZE_MID);
			} 
		}
	
		//render Loop
		unsigned int animSpeed = c.animSpeed/41;
		if(animSpeed == 0) animSpeed = 1;
		for(i=0; i<c.duration*24; i++) {
			render_clearBuffer(new_color(0,0,0));
			
			render_drawSprite(back, new_vector(0,0), 0);
			render_drawSprite(sprte, new_vector(c.spritePos[0],c.spritePos[1]), i/animSpeed);
			renderData(databuffer, textWidth, i, c.dataPos, c.dataAlign, c.dataType);
			renderTime(c.timePos, (i/12)%2);
			
			render_flushBuffer();
			usleep(41666);
		}
	
		delete_sprite(back);
		delete_sprite(sprte);
	}
	
	config_closeFolderSearch();
	render_close();
	return 0;
}

//! Syncronizes clock according to url
void syncTime(char* location, char noCert) {
	secondsDelta = 0;
	if(location) {
		int i,j=0;
		char completeURL[CONFIG_MAX_URL_SIZE*2];
		for(i=0; timeURL[i]; i++) completeURL[j++] = timeURL[i];
		for(i=0; location[i]; i++) completeURL[j++] = location[i];
		completeURL[j++] = 0;
		
		char databuffer[DATABUFFER_SIZE];
		web_request(databuffer, DATABUFFER_SIZE, completeURL, "<span id=ct class=h1>", "</span>", 0, noCert);
		
		i=0;
		int hours = 0;
		for(; databuffer[i] != 0 && databuffer[i] != ':' && databuffer[i] != ' '; i++) hours = hours*10 + (databuffer[i]-48);
		for(; databuffer[i] != 0 && (databuffer[i] < 48 || databuffer[i] > 57); i++);
		int minutes = 0;
		for(; databuffer[i] != 0 && databuffer[i] != ':' && databuffer[i] != ' '; i++) minutes = minutes*10 + (databuffer[i]-48);
		for(; databuffer[i] != 0 && (databuffer[i] < 48 || databuffer[i] > 57); i++);
		int seconds = 0;
		for(; databuffer[i] != 0 && databuffer[i] != ':' && databuffer[i] != ' '; i++) seconds = seconds*10 + (databuffer[i]-48);
		if(databuffer[i+1] == 'p') hours += 12;
		
		secondsDelta = ((hours*60 + minutes)*60 + seconds) - getTimeInSeconds();
	}
}

//! Gets the current time in seconds
int getTimeInSeconds() {
	time_t now = time(NULL);
	struct tm *now_tm = localtime(&now);
	int seconds = ((int)now_tm->tm_hour*60 + (int)now_tm->tm_min)*60 + (int)now_tm->tm_sec;
	seconds += secondsDelta;
	if(seconds < 0) seconds += 24*60*60;
	if(seconds >= 24*60*60) seconds -= 24*60*60;
		
	return seconds;
}

//! Renders the current time
void renderTime(char position, char renderColon) {
	if(position != CONFIG_TIME_POS_NONE) {
		//get time
		int seconds = getTimeInSeconds();
		int hours = seconds/(60*60);
		int minutes = seconds/60 - hours*60;
		if(hours >= 12) hours -= 12;
		if(hours == 0) hours = 12;
		
		//determine digits and widths
		char digit1 = hours/10;
		char digit2 = hours-((hours/10)*10);
		char digit3 = minutes/10;
		char digit4 = minutes-((minutes/10)*10);
		char width = 8;
		if(digit1 == 1) width += 2;
		if(digit2 != 1) width += 2;
		if(digit3 != 1) width += 2;
		if(digit4 != 1) width += 2;
		
		//get position
		int x = 0, y = 0;
		if(position == CONFIG_TIME_POS_CENTER) {
			x=(BUFFER_DIM_X/2)-(width/2)+1; y=6;
		}
		if(position == CONFIG_TIME_POS_TOP_RIGHT) {
			x=BUFFER_DIM_X+1-width;
		}
		if(position == CONFIG_TIME_POS_BOTTOM_LEFT) {
			y=11;
		}
		if(position == CONFIG_TIME_POS_BOTTOM_RIGHT) {
			x=BUFFER_DIM_X+1-width; y=11;
		}
		
		//render
		char buffer[2];
		buffer[1] = 0;
		
		buffer[0] = 48+digit1;
		if(hours/10) render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
		if(digit1 == 1) x+=2;
		
		buffer[0] = 48+digit2;
		render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
		if(digit2 == 1) x+=2;
		else x+=4;
		
		if(renderColon) render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), ":", 0, BUFFER_DIM_X, FONT_SIZE_MID);
		x+=2;
		
		buffer[0] = 48+digit3;
		render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
		if(digit3 == 1) x+=2;
		else x+=4;
		
		buffer[0] = 48+digit4;
		render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
		if(digit4 == 1) x+=2;
		else x+=4;
	}
}

//! Renders the data buffer
void renderData(const char* text, unsigned int textWidth, unsigned int time, signed int* position, unsigned char dataAlign, unsigned char dataType) {
	int i, x = position[0], y = position[1];
	int offset = 0;
	
	if(dataType == CONFIG_DATA_TYPE_NUMBER) {
		for(textWidth=0; text[textWidth]; textWidth++);
		if(dataAlign == CONFIG_DATA_ALIGN_RIGHT) {
			x = (x-(textWidth*4))+1;
		}
		char buffer[2];
		buffer[1] = 0;
		for(i=0; i<textWidth; i++) {
			buffer[0] = text[i];
			if(buffer[0] == 49) render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x+(i*4)+1, y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
			else render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x+(i*4), y), buffer, 0, BUFFER_DIM_X, FONT_SIZE_MID);
		}
	} else {
		if(textWidth > BUFFER_DIM_X) {
			x = 0;
			offset = (time%(textWidth+BUFFER_DIM_X+DATA_SCROLL_DELAY)) - BUFFER_DIM_X;
		} else if(dataAlign == CONFIG_DATA_ALIGN_RIGHT) {
			x= x-textWidth;
		}
		render_drawText(new_color(255,255,255), new_color(0,0,0), new_vector(x, y), text, offset, BUFFER_DIM_X, FONT_SIZE_LARGE);
	}
}
