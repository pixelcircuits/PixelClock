//-----------------------------------------------------------------------------------------
// Title:	Config Resources
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 

const char* config_faceDirectory = "faces";
const char* config_configFile = "config.txt";

const char* config_defaultSpriteName = "sprite";
const char* config_defaultBackgroundName = "background";

const char* systemconfig_systemLocation = "systemLocation";
const char* systemconfig_noURLCheckCert = "noURLCheckCert";

const char* config_labelDuration = "duration";
const char* config_labelTimePos = "timePos";
const char* config_labelbackgroundFile = "backgroundFile";
const char* config_labelSpriteFile = "spriteFile";
const char* config_labelSpritePos = "spritePos";
const char* config_labelClearColor = "clearColor";
const char* config_labelAnimColor = "animColor";
const char* config_labelAnimSpeed = "animSpeed";
const char* config_labelDataURL = "dataURL";
const char* config_labelDataFilterAfter = "dataFilterAfter";
const char* config_labelDataFilterBefore = "dataFilterBefore";
const char* config_labelDataFilterIndex = "dataFilterIndex";
const char* config_labelDataType = "dataType";
const char* config_labelDataPos = "dataPos";
const char* config_labelDataAlign = "dataAlign";

const char* config_timePos_none = "none";
const char* config_timePos_center = "center";
const char* config_timePos_topRight = "topRight";
const char* config_timePos_topLeft = "topLeft";
const char* config_timePos_bottomLeft = "bottomLeft";
const char* config_timePos_bottomRight = "bottomRight";

const char* config_dataAlign_left = "left";
const char* config_dataAlign_right = "right";

const char* config_dataType_text = "text";
const char* config_dataType_number = "number";

DIR* config_searchDir = 0;

//string utils
signed int config_searchString(const char* key, char* string, unsigned int length) {
	unsigned int keyLength=0;
	for(keyLength=0; key[keyLength]; keyLength++);
	
	unsigned int i,j,index;
	for(i=0; i<length-keyLength; i++) {
		char match = 1;
		for(j=0; j<keyLength; j++) if(key[j] != string[j+i]) { match=0; break; }
		if(match>0) {
			index = i+keyLength;
			for(; index<length; index++) {
				if(string[index] == ':') return index+1;
				if(string[index] != ' ' && string[index] != '"' && string[index] != '\'') return -1;
			}
		}
	}
	
	return -1;
}
unsigned int config_getStringInt(signed int* value, char* string, unsigned int index, unsigned int length) {
	signed char sign = 1;
	for(; index<length; index++) {
		if(string[index] == '-') sign = -1;
		else if(string[index]>=48 && string[index]<=57) break;
		else if(string[index] != ' ' && string[index] != ',' && string[index] != '[') return index;
	}
	
	signed int number = 0;
	for(; index<length; index++) {
		if(string[index]>=48 && string[index]<=57) number = (number*10) + (string[index]-48);
		else break;
	}
	
	*value = number*sign;
	return index;
}
unsigned int config_getStringArrayInt(signed int* value, unsigned char size, char* string, unsigned int index, unsigned int length) {
	int i;
	for(i=0; i<size; i++) index = config_getStringInt(&(value[i]), string, index, length);
	return index;
}
unsigned int config_getStringText(char* value, unsigned int maxSize, char* string, unsigned int index, unsigned int length) {
	value[0] = 0;
	char encapsulatingChar;
	for(; index<length; index++) {
		if(string[index] == '\'' || string[index] == '"') {
			encapsulatingChar = string[index];
			break;
		}
		else if(string[index] != ' ') return index;
	}
	index++;
	
	unsigned int i;
	for(i=0; i<maxSize-1 && (index+i)<length; i++) {
		if(string[index+i] == encapsulatingChar) break;
		value[i] = string[index+i];
	}
	value[i] = 0;
	
	return index+i;
}
unsigned char config_compareStrings(const char* a, const char* b) {
	unsigned int i;
	for(i=0; ; i++) {
		if(a[i] != b[i]) return 0;
		if(a[i] == 0) break;
	}
	return 1;
}

//! Gets config file data.
systemConfig config_getSystemConfig()
{
	systemConfig c;
	c.systemLocation[0] = 0;
	c.noURLCheckCert = 0;
	
	FILE *fp;
	fp = fopen(config_configFile, "rb");
	if(fp != NULL){
		
		//read file into memory
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* fileData = malloc(fsize);
		fread(fileData, fsize, 1, fp);
		
		//look for each config items
		signed int index;
		index = config_searchString(systemconfig_systemLocation, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.systemLocation, CONFIG_MAX_URL_SIZE, fileData, index, fsize);
		}
		index = config_searchString(systemconfig_noURLCheckCert, fileData, fsize);
		if(index > -1){ 
			signed int noCert;
			config_getStringInt(&noCert, fileData, index, fsize);
			c.noURLCheckCert = noCert;
		}
		
		//free memory and close file
		free(fileData);
		fclose(fp);
	}
	
	return c;
}

//! Gets config file data.
config config_getConfig(const char* foldername)
{
	int i;
	config c;
	c.duration = 5;
	c.timePos = CONFIG_TIME_POS_TOP_RIGHT;
	for(i=0; config_defaultBackgroundName[i]; i++) c.backgroundFile[i] = config_defaultBackgroundName[i]; 
	c.backgroundFile[i] = 0; 
	for(i=0; config_defaultSpriteName[i]; i++) c.spriteFile[i] = config_defaultSpriteName[i]; 
	c.spriteFile[i] = 0; 
	c.spritePos[0] = 0;	c.spritePos[1] = 0;
	c.clearColor[0] = 255; c.clearColor[1] = 0; c.clearColor[2] = 255;
	c.animColor[0] = 0; c.animColor[1] = 255; c.animColor[2] = 255;
	c.animSpeed = 200;
	c.dataURL[0] = 0;
	c.dataFilterBefore[0] = 0;
	c.dataFilterAfter[0] = 0;
	c.dataFilterIndex = 0;
	c.dataType = CONFIG_DATA_TYPE_TEXT;
	c.dataPos[0] = 0; c.dataPos[1] = 0;
	c.dataAlign = CONFIG_DATA_ALIGN_LEFT;
	
	FILE *fp;
	char buffer[CONFIG_MAX_FILENAME_SIZE*4];
	fp = fopen(config_constructFilePath(buffer, foldername, config_configFile), "rb");
	if(fp != NULL){
		
		//read file into memory
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* fileData = malloc(fsize);
		fread(fileData, fsize, 1, fp);
		
		//look for each config items
		signed int index;
		index = config_searchString(config_labelDuration, fileData, fsize);
		if(index > -1){ 
			config_getStringInt(&(c.duration), fileData, index, fsize);
		}
		index = config_searchString(config_labelTimePos, fileData, fsize);
		if(index > -1){ 
			char buffer[255];
			config_getStringText(buffer, 255, fileData, index, fsize);
			if(config_compareStrings(buffer, config_timePos_none)) c.timePos = CONFIG_TIME_POS_NONE;
			if(config_compareStrings(buffer, config_timePos_center)) c.timePos = CONFIG_TIME_POS_CENTER;
			if(config_compareStrings(buffer, config_timePos_topRight)) c.timePos = CONFIG_TIME_POS_TOP_RIGHT;
			if(config_compareStrings(buffer, config_timePos_topLeft)) c.timePos = CONFIG_TIME_POS_TOP_LEFT;
			if(config_compareStrings(buffer, config_timePos_bottomLeft)) c.timePos = CONFIG_TIME_POS_BOTTOM_LEFT;
			if(config_compareStrings(buffer, config_timePos_bottomRight)) c.timePos = CONFIG_TIME_POS_BOTTOM_RIGHT;
		}
		index = config_searchString(config_labelbackgroundFile, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.backgroundFile, CONFIG_MAX_FILENAME_SIZE, fileData, index, fsize);
		}
		index = config_searchString(config_labelSpriteFile, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.spriteFile, CONFIG_MAX_FILENAME_SIZE, fileData, index, fsize);
		}
		index = config_searchString(config_labelSpritePos, fileData, fsize);
		if(index > -1){ 
			config_getStringArrayInt(c.spritePos, 2, fileData, index, fsize);
		}
		index = config_searchString(config_labelClearColor, fileData, fsize);
		if(index > -1){ 
			signed int data[3]; data[0]=c.clearColor[0]; data[1]=c.clearColor[1]; data[2]=c.clearColor[2];
			config_getStringArrayInt(data, 3, fileData, index, fsize);
			c.clearColor[0]=data[0]; c.clearColor[1]=data[1]; c.clearColor[2]=data[2];
		}
		index = config_searchString(config_labelAnimColor, fileData, fsize);
		if(index > -1){ 
			signed int data[3]; data[0]=c.animColor[0]; data[1]=c.animColor[1]; data[2]=c.animColor[2];
			config_getStringArrayInt(data, 3, fileData, index, fsize);
			c.animColor[0]=data[0]; c.animColor[1]=data[1]; c.animColor[2]=data[2];
		}
		index = config_searchString(config_labelAnimSpeed, fileData, fsize);
		if(index > -1){ 
			config_getStringInt(&(c.animSpeed), fileData, index, fsize);
		}
		index = config_searchString(config_labelDataURL, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.dataURL, CONFIG_MAX_URL_SIZE, fileData, index, fsize);
		}
		index = config_searchString(config_labelDataFilterAfter, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.dataFilterAfter, CONFIG_MAX_URL_FILTER_SIZE, fileData, index, fsize);
		}
		index = config_searchString(config_labelDataFilterBefore, fileData, fsize);
		if(index > -1){ 
			config_getStringText(c.dataFilterBefore, CONFIG_MAX_URL_FILTER_SIZE, fileData, index, fsize);
		}
		index = config_searchString(config_labelDataFilterIndex, fileData, fsize);
		if(index > -1){ 
			config_getStringInt(&(c.dataFilterIndex), fileData, index, fsize);
		}
		index = config_searchString(config_labelDataType, fileData, fsize);
		if(index > -1){ 
			char buffer[255];
			config_getStringText(buffer, 255, fileData, index, fsize);
			if(config_compareStrings(buffer, config_dataType_text)) c.dataType = CONFIG_DATA_TYPE_TEXT;
			if(config_compareStrings(buffer, config_dataType_number)) c.dataType = CONFIG_DATA_TYPE_NUMBER;
		}
		index = config_searchString(config_labelDataPos, fileData, fsize);
		if(index > -1){ 
			config_getStringArrayInt(c.dataPos, 2, fileData, index, fsize);
		}
		index = config_searchString(config_labelDataAlign, fileData, fsize);
		if(index > -1){ 
			char buffer[255];
			config_getStringText(buffer, 255, fileData, index, fsize);
			if(config_compareStrings(buffer, config_dataAlign_left)) c.dataAlign = CONFIG_DATA_ALIGN_LEFT;
			if(config_compareStrings(buffer, config_dataAlign_right)) c.dataAlign = CONFIG_DATA_ALIGN_RIGHT;
		}
		
		//free memory and close file
		free(fileData);
		fclose(fp);
	}
	
	return c;
}

//! Gets next folder in the search directory.
void config_getNextFolder(char* foldername)
{
	unsigned int i;
	foldername[0] = 0;
	if(config_searchDir == 0) {
		config_searchDir = opendir(config_faceDirectory);
	}
	
	if(config_searchDir) {
		char reset = 0;
		while(1) {
			struct dirent* dir = readdir(config_searchDir);
			if(dir) {
			
				// found folder
				if(dir->d_type == DT_DIR && dir->d_name[0] != '.' && dir->d_name[1] != '.') {
					for(i=0; dir->d_name[i]; i++) foldername[i] =  dir->d_name[i];
					foldername[i] = 0;
					break;
				}
			}
			
			// restart search
			if(dir == NULL) {
				if(reset > 0) break;
				
				closedir(config_searchDir);
				config_searchDir = opendir(config_faceDirectory);
				reset = 1;
			}
		}
	}
}

//! Closes folder search.
void config_closeFolderSearch()
{
	if(config_searchDir) {
		closedir(config_searchDir);
		config_searchDir = 0;
	}
}

//! Helps construct full file path.
char* config_constructFilePath(char* buffer, const char* foldername, const char* filename)
{
	unsigned int offset=0;
	unsigned int i=0;
	for(; config_faceDirectory[i]; i++) buffer[i] = config_faceDirectory[i];
	buffer[i++] = '/';
	
	offset = i;
	for(; foldername[i-offset]; i++) buffer[i] = foldername[i-offset];
	buffer[i++] = '/';
	
	offset = i;
	for(; filename[i-offset]; i++) buffer[i] = filename[i-offset];
	buffer[i++] = 0;
	
	return buffer;
}
