//-----------------------------------------------------------------------------------------
// Title:	Web Scrapper
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef WEB_H
#define WEB_H

//! Extracts data from a url.
void web_request(char* buffer, unsigned int bufferSize, const char* url, const char* afterFilter, const char* beforeFilter, unsigned int index, char noCert);

#endif
