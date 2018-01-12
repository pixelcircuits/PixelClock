//-----------------------------------------------------------------------------------------
// Title:	Video Renderer
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef RENDER_H
#define RENDER_H

#define BUFFER_DIM_X 32
#define BUFFER_DIM_Y 16

#define FONT_SIZE_SMALL 0
#define FONT_SIZE_MID 1
#define FONT_SIZE_LARGE 2

typedef struct color_s {
    unsigned char R;
    unsigned char G;
    unsigned char B;
} color;
color new_color(unsigned char R, unsigned char G, unsigned char B);

typedef struct vector_s {
    signed int X;
    signed int Y;
} vector;
vector new_vector(signed int X, signed int Y);

typedef struct sprite_s {
	unsigned int width;
	unsigned int height;
	unsigned int numFrames;
	color* colorData;
} sprite;
sprite new_sprite(const char* filename, unsigned char* clearColor, unsigned char* animColor);
void delete_sprite(sprite s);


//! Initialize the renderer.
void render_init();

//! Close the renderer.
void render_close();

//! Draw a point to the video buffer.
void render_drawPoint(color c, vector v);

//! Draw a line to the video buffer.
void render_drawLine(color c, vector v0, vector v1);

//! Draw a rectangle to the video buffer.
void render_drawRect(color outlineColor, color fillColor, vector v0, vector v1);

//! Draw text to the video buffer.
void render_drawText(color c0, color c1, vector v, const char* text, signed int offset, unsigned int limit, unsigned char fontSize);

//! Draw a sprite to the video buffer.
void render_drawSprite(sprite s, vector v, unsigned int frame);

//! Gets the width of rendered text.
unsigned int render_getTextWidth(const char* text, unsigned char fontSize);

//! Get a color from the video buffer.
color render_colorAt(vector pos);

//! Clear the video buffer to the specified color.
void render_clearBuffer(color color);

//! Copy video buffer to the display.
void render_flushBuffer();

//! Set display brightness.
void render_setBrightness(unsigned char brightness);

//! Get display brightness.
unsigned char render_getBrightness();

#endif
