//-----------------------------------------------------------------------------------------
// Title:	Config Resources
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAX_FILENAME_SIZE 255
#define CONFIG_MAX_URL_SIZE 4096
#define CONFIG_MAX_URL_FILTER_SIZE 4096

#define CONFIG_TIME_POS_NONE 0
#define CONFIG_TIME_POS_CENTER 1
#define CONFIG_TIME_POS_TOP_LEFT 2
#define CONFIG_TIME_POS_TOP_RIGHT 3
#define CONFIG_TIME_POS_BOTTOM_LEFT 4
#define CONFIG_TIME_POS_BOTTOM_RIGHT 5

#define CONFIG_DATA_ALIGN_LEFT 0
#define CONFIG_DATA_ALIGN_RIGHT 1

#define CONFIG_DATA_TYPE_TEXT 0
#define CONFIG_DATA_TYPE_NUMBER 1

typedef struct config_s {
	unsigned int duration;
	unsigned char timePos;
	char backgroundFile[CONFIG_MAX_FILENAME_SIZE];

	char spriteFile[CONFIG_MAX_FILENAME_SIZE];
	signed int spritePos[2];
	unsigned char clearColor[3];
	unsigned char animColor[3];
	unsigned int animSpeed;

	char dataURL[CONFIG_MAX_URL_SIZE];
	char dataFilterBefore[CONFIG_MAX_URL_FILTER_SIZE];
	char dataFilterAfter[CONFIG_MAX_URL_FILTER_SIZE];
	unsigned int dataFilterIndex;
	unsigned char dataType;
	signed int dataPos[2];
	unsigned char dataAlign;
} config;

typedef struct systemConfig_s {
	char systemLocation[CONFIG_MAX_URL_SIZE];
	char noURLCheckCert;
} systemConfig;


//! Gets config file data.
systemConfig config_getSystemConfig();

//! Gets config file data.
config config_getConfig(const char* foldername);

//! Gets next folder in the search directory.
void config_getNextFolder(char* foldername);

//! Closes folder search.
void config_closeFolderSearch();

//! Helps construct full file path.
char* config_constructFilePath(char* buffer, const char* foldername, const char* filename);

#endif
