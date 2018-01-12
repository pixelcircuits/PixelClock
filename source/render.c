//-----------------------------------------------------------------------------------------
// Title:	Video Renderer
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "render.h"
#include "fonts.h"
#include "images.h"
#include "pixeldisplay.h"
#include "pixeldisplay.c"

#define ABS(a) (((a)<0) ? -(a) : (a))
#define ZSGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)
#define CLEAR_COLOR 1


//! Initializes the renderer.
void render_init()
{
	pixelDisplay_init();
}

//! Closes the renderer.
void render_close()
{
	pixelDisplay_close(PIXEL_DISPLAY_CLOSE_ACTION_OFF);
}

//! Draw a point to the video buffer.
void render_drawPoint(color c, vector v)
{	
	pixelDisplay_write(v.X, v.Y, c.R, c.G, c.B);
}

//! Draw a line to the video buffer.
void render_drawLine(color c, vector v0, vector v1)
{
	signed int xd, yd;
    signed int dx = v1.X - v0.X;
    signed int dy = v1.Y - v0.Y;
    signed int ax = ABS(dx) << 1;
    signed int ay = ABS(dy) << 1;
    signed int sx = ZSGN(dx);
    signed int sy = ZSGN(dy);
    signed int x = v0.X;
    signed int y = v0.Y;

	//x dominant
    if (ax >= ay) 
    {
        yd = ay - (ax >> 1);
        for (;;) {
            render_drawPoint(c, new_vector(x, y));
            if (x == v1.X) return;

            if (yd >= 0) {
                y += sy;
                yd -= ax;
            }

            x += sx;
            yd += ay;
        }
    }
	//y dominant
    else if (ay >= ax) 
    {
        xd = ax - (ay >> 1);
        for (;;) {
            render_drawPoint(c, new_vector(x, y));
            if (y == v1.Y) return;

            if (xd >= 0) {
                x += sx;
                xd -= ay;
            }

            y += sy;
            xd += ax;
        }
    }
}

//! Draw a rectangle to the video buffer.
void render_drawRect(color outlineColor, color fillColor, vector v0, vector v1)
{
	int xmin = v0.X;
    int xmax = v1.X;
	if(xmin > xmax) {
		xmin = v1.X;
		xmax = v0.X;
	}

	int ymin = v0.Y;
    int ymax = v1.Y;
	if(ymin > ymax) {
		ymin = v1.Y;
		ymax = v0.Y;
	}

	//draw the outline
	for (int x = xmin; x < xmax; x++) {
		render_drawPoint(outlineColor, new_vector(x, ymin));
		render_drawPoint(outlineColor, new_vector(x, ymax-1));

		for(int y = ymin+1; y<ymax-1; y++) render_drawPoint(fillColor, new_vector(x, y));
    }
	for (int y = ymin; y < ymax; y++) {
		render_drawPoint(outlineColor, new_vector(xmin, y));
		render_drawPoint(outlineColor, new_vector(xmax-1, y));
    }
}

//! Draw text to the video buffer.
void render_drawText(color c0, color c1, vector v, const char* text, signed int offset, unsigned int limit, unsigned char fontSize)
{
	//if the same color, then don't draw outline
	char outline = 1;
	if(c0.R == c1.R && c0.G == c1.G && c0.B == c1.B) outline = 0;
	
	//find starting x
	signed int x = 0;
	if(offset < 0) {
		x = -offset;
		offset = 0;
	}

	//get height for all characters
	unsigned int cHeight = SMALL_FONT_HIEGHT;
	if(fontSize == 1) cHeight = MEDIUM_FONT_HIEGHT;
	if(fontSize == 2) cHeight = LARGE_FONT_HIEGHT;

	//loop through characters
	unsigned int c = 0;
	unsigned char txt = text[0];
	while(txt!=0 && x<limit) {
		
		//get character index form ascii value
		unsigned char i = 0;
		if(txt>=32 && txt <=126) i = txt-32;

		//get characters width
		unsigned char cWidth = SMALL_FONT_WIDTHS[i+1] - SMALL_FONT_WIDTHS[i];
		if(fontSize == 1) cWidth = MEDIUM_FONT_WIDTHS[i+1] - MEDIUM_FONT_WIDTHS[i];
		if(fontSize == 2) cWidth = LARGE_FONT_WIDTHS[i+1] - LARGE_FONT_WIDTHS[i];

		//run along the border of the character before rendering
		if(outline>0) {
			int tempX = x + offset;
			for(signed int w=offset; w<cWidth && tempX<(signed int)limit; w++) {
				if(fontSize == 1) {
					int arrayLocation = MEDIUM_FONT_WIDTHS[i];
					if(MEDIUM_FONT[MEDIUM_FONT_ROW*(cHeight-1) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, cHeight+v.Y));
					if(MEDIUM_FONT[MEDIUM_FONT_ROW*(0) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, -1+v.Y));
				} else if(fontSize == 2) {
					int arrayLocation = LARGE_FONT_WIDTHS[i];
					if(LARGE_FONT[LARGE_FONT_ROW*(cHeight-1) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, cHeight+v.Y));
					if(LARGE_FONT[LARGE_FONT_ROW*(0) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, -1+v.Y));
				} else {
					int arrayLocation = SMALL_FONT_WIDTHS[i];
					if(SMALL_FONT[SMALL_FONT_ROW*(cHeight-1) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, cHeight+v.Y));
					if(SMALL_FONT[SMALL_FONT_ROW*(0) + arrayLocation + w]) render_drawPoint(c1, new_vector(tempX+v.X-offset, -1+v.Y));
				}
				tempX++;
			}
			for(signed int h=0; h<cHeight; h++) {
				if(fontSize == 1) {
					int arrayLocation = MEDIUM_FONT_WIDTHS[i];
					if(!offset && MEDIUM_FONT[MEDIUM_FONT_ROW*h + arrayLocation + 0]) render_drawPoint(c1, new_vector(x+v.X-1, h+v.Y));
					if((offset-cWidth)<=0 && x+cWidth<limit && MEDIUM_FONT[MEDIUM_FONT_ROW*h + arrayLocation + cWidth-1]) render_drawPoint(c1, new_vector(x+v.X+cWidth-offset, h+v.Y));
				} else if(fontSize == 2) {
					int arrayLocation = LARGE_FONT_WIDTHS[i];
					if(!offset && LARGE_FONT[LARGE_FONT_ROW*h + arrayLocation + 0]) render_drawPoint(c1, new_vector(x+v.X-1, h+v.Y));
					if((offset-cWidth)<=0 && x+cWidth<limit && LARGE_FONT[LARGE_FONT_ROW*h + arrayLocation + cWidth-1]) render_drawPoint(c1, new_vector(x+v.X+cWidth-offset, h+v.Y));
				} else {
					int arrayLocation = SMALL_FONT_WIDTHS[i];
					if(offset==0 && SMALL_FONT[SMALL_FONT_ROW*h + arrayLocation + 0]) render_drawPoint(c1, new_vector(x+v.X-1, h+v.Y));
					if((offset-cWidth)<=0 && x+cWidth<limit && SMALL_FONT[SMALL_FONT_ROW*h + arrayLocation + cWidth-1]) render_drawPoint(c1, new_vector(x+v.X+cWidth-offset, h+v.Y));
				}
			}
		}

		//loop through char width
		for(int w=0; w<cWidth && x<limit; w++) {
			
			//don't start rendering until the offset is gone
			if(offset) {
				offset--;
			} else {
				
				//loop through char height
				for(unsigned int h=0; h<cHeight && x<limit; h++) {
					
					//draw point
					if(fontSize == 1) {
						int arrayLocation = MEDIUM_FONT_WIDTHS[i];
						if(MEDIUM_FONT[MEDIUM_FONT_ROW*h + arrayLocation + w]) render_drawPoint(c0, new_vector(x+v.X, h+v.Y));
						else if(outline>0) {
							if(    (h>0 && MEDIUM_FONT[MEDIUM_FONT_ROW*(h-1) + arrayLocation + w])
								|| (h<(cHeight-1) && MEDIUM_FONT[MEDIUM_FONT_ROW*(h+1) + arrayLocation + w])
								|| (w>0 && MEDIUM_FONT[MEDIUM_FONT_ROW*h + arrayLocation + (w-1)])
								|| (w<(cWidth-1) && MEDIUM_FONT[MEDIUM_FONT_ROW*h + arrayLocation + (w+1)]))
								render_drawPoint(c1, new_vector(x+v.X, h+v.Y));
						}
					} else if(fontSize == 2) {
						int arrayLocation = LARGE_FONT_WIDTHS[i];
						if(LARGE_FONT[LARGE_FONT_ROW*h + arrayLocation + w]) render_drawPoint(c0, new_vector(x+v.X, h+v.Y));
						else if(outline>0) {
							if(    (h>0 && LARGE_FONT[LARGE_FONT_ROW*(h-1) + arrayLocation + w])
								|| (h<(cHeight-1) && LARGE_FONT[LARGE_FONT_ROW*(h+1) + arrayLocation + w])
								|| (w>0 && LARGE_FONT[LARGE_FONT_ROW*h + arrayLocation + (w-1)])
								|| (w<(cWidth-1) && LARGE_FONT[LARGE_FONT_ROW*h + arrayLocation + (w+1)]))
								render_drawPoint(c1, new_vector(x+v.X, h+v.Y));
						}
					} else {
						int arrayLocation = SMALL_FONT_WIDTHS[i];
						if(SMALL_FONT[SMALL_FONT_ROW*h + arrayLocation + w]) render_drawPoint(c0, new_vector(x+v.X, h+v.Y));
						else if(outline>0) {
							if(    (h>0 && SMALL_FONT[SMALL_FONT_ROW*(h-1) + arrayLocation + w])
								|| (h<(cHeight-1) && SMALL_FONT[SMALL_FONT_ROW*(h+1) + arrayLocation + w])
								|| (w>0 && SMALL_FONT[SMALL_FONT_ROW*h + arrayLocation + (w-1)])
								|| (w<(cWidth-1) && SMALL_FONT[SMALL_FONT_ROW*h + arrayLocation + (w+1)]))
								render_drawPoint(c1, new_vector(x+v.X, h+v.Y));
						}
					}
				}

				//incriment the x location
				x++;
			}
		}
		
		//don't start rendering until the offset is gone
		if(offset>0) offset--;
		else x++;

		//incriment to the next character in the string
		c++;
		txt = text[c];
	}
}

//! Draw a sprite to the video buffer.
void render_drawSprite(sprite s, vector v, unsigned int frame)
{
	unsigned int h,w;
	if(s.width>0 && s.height>0 && s.colorData) {
		frame = frame%s.numFrames;
		
		for(h=0; h<s.height; h++) {
			for(w=0; w<s.width; w++) {
				
				unsigned int index = (s.width*s.height*frame) + (h*s.width) + w;
				if(s.colorData[index].R != CLEAR_COLOR || s.colorData[index].G != CLEAR_COLOR || s.colorData[index].B != CLEAR_COLOR) {
					render_drawPoint(s.colorData[index], new_vector(w+v.X, h+v.Y));
				}
			}
		}
	}
}

//! Gets the width of rendered text.
unsigned int render_getTextWidth(const char* text, unsigned char fontSize)
{
	unsigned int textWidth = 0;
	const char* txt = text;
	while(*txt != 0) {
		//get character index form ascii value
		unsigned char i = 0;
		if((*txt)>=32 && (*txt) <=126) i = (*txt)-32;

		//get width of character
		unsigned int cWidth = SMALL_FONT_WIDTHS[i+1] - SMALL_FONT_WIDTHS[i];
		if(fontSize==1) cWidth = MEDIUM_FONT_WIDTHS[i+1] - MEDIUM_FONT_WIDTHS[i];
		if(fontSize==2) cWidth = LARGE_FONT_WIDTHS[i+1] - LARGE_FONT_WIDTHS[i];

		//increase textWidth
		textWidth += cWidth+1;
		txt++;
	}
	textWidth--;
	return textWidth;
}

//! Get a color from the buffer.
color render_colorAt(vector pos)
{
	if(pos.X<0) pos.X = 0;
	if(pos.X>BUFFER_DIM_X-1) pos.X=BUFFER_DIM_X-1;

	if(pos.Y<0) pos.Y = 0;
	if(pos.Y>BUFFER_DIM_Y-1) pos.Y=BUFFER_DIM_Y-1;
	
	color color;
	pixelDisplay_read(pos.X, pos.Y, &(color.R), &(color.G), &(color.B));
	return color;
}

//! Clear the video buffer to the specified color.
void render_clearBuffer(color color)
{
	pixelDisplay_clear(color.R, color.G, color.B);
}

//! Copy video buffer to the display.
void render_flushBuffer()
{
	pixelDisplay_flush();
}

//! Set display brightness.
void render_setBrightness(unsigned char brightness)
{
	pixelDisplay_setBrightness(brightness);
}

//! Get display brightness.
unsigned char render_getBrightness()
{
	return pixelDisplay_getBrightness();
}

//! Struct helper functions.
color new_color(unsigned char R, unsigned char G, unsigned char B) {
	color c; c.R = R; c.G = G; c.B = B;
	return c;
}
vector new_vector(signed int X, signed int Y) {
	vector v; v.X = X; v.Y = Y;
	return v;
}
sprite new_sprite(const char* filename, unsigned char* clearColor, unsigned char* animColor) {
	unsigned int h,w;
	sprite s;
	s.width = 0;
	s.height = 0;
	s.numFrames = 0;
	s.colorData = 0;
	
	unsigned int width, height;
	unsigned char* colorData = images_getImageData(filename, &width, &height);
	if(colorData) {
		
		//find the max width for each frame
		unsigned int frameWidth = width;
		unsigned int numFrames = 1;
		if(animColor) {
			for(w=0; w<width; w++) {
				if(colorData[w*3] == animColor[0] && colorData[w*3 + 1] == animColor[1] && colorData[w*3 + 2] == animColor[2]) {
					
					//ANIM_RULE: frame markers cannot be at the beginning or end
					if(w == 0) break;

					//ANIM_RULE: markers must logically divide into image width
					if((width+1)%(w+1) != 0) break;

					//frame width and number of frames can be determined
					frameWidth = w;
					numFrames = (width+1)/(w+1);
					break;
				}
			}
		}
		
		//allocate data for the sprite
		s.width = frameWidth;
		s.height = height;
		s.numFrames = numFrames;
		s.colorData = malloc(sizeof(color)*(frameWidth*height*numFrames));
		
		//copy data into sprite
		for(h=0; h<height; h++) {
			int currentframe = 0;
			for( w=0; w<width; w++) {
				if(w%(frameWidth+1) == frameWidth) {
					currentframe++;
				} else {
					unsigned int frameW = w-currentframe*(frameWidth+1);
					unsigned int colorIndex = currentframe*frameWidth*height + h*frameWidth + frameW;
					s.colorData[colorIndex].R = colorData[(((height-1)-h)*width + w)*3 + 0];
					s.colorData[colorIndex].G = colorData[(((height-1)-h)*width + w)*3 + 1];
					s.colorData[colorIndex].B = colorData[(((height-1)-h)*width + w)*3 + 2];
					
					//move color if on the color flagged as clear
					if(s.colorData[colorIndex].R == CLEAR_COLOR && s.colorData[colorIndex].G == CLEAR_COLOR && s.colorData[colorIndex].B == CLEAR_COLOR) {
						s.colorData[colorIndex].R = CLEAR_COLOR-1;
						s.colorData[colorIndex].G = CLEAR_COLOR-1;
						s.colorData[colorIndex].B = CLEAR_COLOR-1;
					}
					
					//check if color should be flagged as clear
					if(clearColor && s.colorData[colorIndex].R == clearColor[0] && s.colorData[colorIndex].G == clearColor[1] && s.colorData[colorIndex].B == clearColor[2]) {
						s.colorData[colorIndex].R = CLEAR_COLOR;
						s.colorData[colorIndex].G = CLEAR_COLOR;
						s.colorData[colorIndex].B = CLEAR_COLOR;
					}
				}
			}
		}
		
		//remove raw color data from memory
		free(colorData);
	}
	
	return s;
}
void delete_sprite(sprite s) {
	free(s.colorData);
}
