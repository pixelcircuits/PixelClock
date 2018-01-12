//-----------------------------------------------------------------------------------------
// Title:	Image Resources
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "images.h"
#include "upng.h"
#include "upng.c"
#include <stdio.h>
#include <stdlib.h>

#define FILE_EXTENSION_NONE 0
#define FILE_EXTENSION_BMP 1
#define FILE_EXTENSION_PNG 2

//decode utils
unsigned char* images_decodeBMP(FILE* fp, unsigned int* width, unsigned int* height);
unsigned char* images_decodePNG(FILE* fp, unsigned int* width, unsigned int* height);
unsigned int read_u32(const char* in){
	return ((unsigned int)in[0]) | ((unsigned int)in[1])<<8 | ((unsigned int)in[2])<<16 | ((unsigned int)in[3])<<24;
}
unsigned int read_u16(const char* in){
	return ((unsigned int)in[0]) | ((unsigned int)in[1])<<8;
}

//file extension utils
char images_getFileExtension(const char* filename) {
	unsigned int i;
	for(i=0; filename[i]; i++) {
		if(filename[i] == '.' && (filename[i+1] == 'P' || filename[i+1] == 'p') && (filename[i+2] == 'N' || filename[i+2] == 'n')
			&& (filename[i+3] == 'G' || filename[i+3] == 'g') && filename[i+4] == 0) return FILE_EXTENSION_PNG;
		if(filename[i] == '.' && (filename[i+1] == 'B' || filename[i+1] == 'b') && (filename[i+2] == 'M' || filename[i+2] == 'm')
			&& (filename[i+3] == 'P' || filename[i+3] == 'p') && filename[i+4] == 0) return FILE_EXTENSION_BMP;
	}
	return FILE_EXTENSION_NONE;
}
char* images_addFileExtension(char* buffer, const char* filename, char extension) {
	unsigned int i;
	for(i=0; filename[i]; i++) buffer[i] = filename[i];
	buffer[i++] = '.';
	if(extension == FILE_EXTENSION_BMP) { buffer[i++] = 'b'; buffer[i++] = 'm'; buffer[i++] = 'p'; }
	if(extension == FILE_EXTENSION_PNG) { buffer[i++] = 'p'; buffer[i++] = 'n'; buffer[i++] = 'g'; }
	buffer[i++] = 0;
	return buffer;
}

//! Gets raw image data from file.
unsigned char* images_getImageData(const char* filename, unsigned int* width, unsigned int* height)
{
	unsigned char* colorData = 0;
	char extension = images_getFileExtension(filename);
	
	if(extension == FILE_EXTENSION_NONE) {
		
		//no extension, try them all until one works
		FILE *fp;
		char buffer[512];
		if(colorData == 0) {
			fp = fopen(images_addFileExtension(buffer, filename, FILE_EXTENSION_BMP), "rb");
			if(fp != NULL){
				colorData = images_decodeBMP(fp, width, height);
				fclose(fp);
			}
		}
		if(colorData == 0) {
			fp = fopen(images_addFileExtension(buffer, filename, FILE_EXTENSION_PNG), "rb");
			if(fp != NULL){
				colorData = images_decodePNG(fp, width, height);
				fclose(fp);
			}
		}
	} else {
		
		FILE *fp;
		fp = fopen(filename, "rb");
		if(fp != NULL){
			if(extension == FILE_EXTENSION_BMP) colorData = images_decodeBMP(fp, width, height);
			if(extension == FILE_EXTENSION_PNG) colorData = images_decodePNG(fp, width, height);
			fclose(fp);
		}
	}
	
	return colorData;
}

//! Decodes a bmp file.
unsigned char* images_decodeBMP(FILE* fp, unsigned int* width, unsigned int* height)
{
	unsigned int h,w;
	
	//read file into memory
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* fileData = malloc(fsize);
	fread(fileData, fsize, 1, fp);

	//make sure this is a bitmap file
	if(fileData[0] != 'B' && fileData[1] != 'M')  return 0;
	unsigned int filesize = read_u32(&fileData[2]);
	unsigned int offset = read_u32(&fileData[10]);
	unsigned int headersize = read_u32(&fileData[14]);

	//right now we only support BITMAPINFOHEADER format
	if(headersize != 40) return 0;
	unsigned int imageWidth = read_u32(&fileData[18]);
	unsigned int imageHeight = read_u32(&fileData[22]);
	unsigned short bitdepth = read_u16(&fileData[28]);
	unsigned int compression = read_u32(&fileData[30]);
	unsigned int bytedepth = bitdepth/8;
	unsigned int padding = (imageWidth*bytedepth)%4;
	if(padding) padding = 4-padding;

	//right now we don't support any kind of compression
	if(compression != 0) return 0;
	
	//right now we only support 24bit color depth
	if(bitdepth != 24) return 0;
	
	//allocate the data array to return
	unsigned int colorDataIndex = 0;
	unsigned char* colorData = malloc(imageWidth*imageHeight*3);

	//decode the color data
	if(bitdepth == 24) {
		for(h=0; h<imageHeight; h++) {
			for(w=0; w<imageWidth; w++) {
				colorData[colorDataIndex++] = fileData[offset + (w + (h * imageWidth)) * bytedepth+2];
				colorData[colorDataIndex++] = fileData[offset + (w + (h * imageWidth)) * bytedepth+1];
				colorData[colorDataIndex++] = fileData[offset + (w + (h * imageWidth)) * bytedepth];
			}
			offset += padding;
		}
	}
	
	//remove file from memory
	free(fileData);
	
	//set data and return
	*width = imageWidth;
	*height = imageHeight;
	return colorData;
}

//! Decodes a png file.
unsigned char* images_decodePNG(FILE* fp, unsigned int* width, unsigned int* height)
{
	unsigned int h,w;
	unsigned char* colorData = 0;
	
	//read file into memory
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* fileData = malloc(fsize);
	fread(fileData, fsize, 1, fp);
	
	//decode png
	upng_t* upng = upng_new_from_bytes(fileData, fsize);
	if (upng != NULL) {
		upng_decode(upng);
		if (upng_get_error(upng) == UPNG_EOK) {
			
			unsigned int imageWidth = upng_get_width(upng);
			unsigned int imageHeight = upng_get_height(upng);
			*width = imageWidth;
			*height = imageHeight;
			const unsigned char* buffer = upng_get_buffer(upng);
			colorData = malloc(imageWidth*imageHeight*3);
			if(upng_get_components(upng) == 4) {
				for(h=0; h<imageHeight; h++) {
					for(w=0; w<imageWidth; w++) {
						colorData[(w + (h * imageWidth))*3 + 0] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*4 + 0];
						colorData[(w + (h * imageWidth))*3 + 1] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*4 + 1];
						colorData[(w + (h * imageWidth))*3 + 2] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*4 + 2];
					}
				}
			}
			else if(upng_get_components(upng) == 3) {
				for(h=0; h<imageHeight; h++) {
					for(w=0; w<imageWidth; w++) {
						colorData[(w + (h * imageWidth))*3 + 0] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*3 + 0];
						colorData[(w + (h * imageWidth))*3 + 1] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*3 + 1];
						colorData[(w + (h * imageWidth))*3 + 2] = buffer[(w + (((imageHeight-1)-h) * imageWidth))*3 + 2];
					}
				}
			}
			
		}
		upng_free(upng);
	}
	
	//remove file from memory
	free(fileData);
	return colorData;
}
