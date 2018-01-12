//-----------------------------------------------------------------------------------------
// Title:	Web Scrapper
// Program: Clock UI
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "web.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char* web_baseCommand = "wget -qO- ";
const char* web_baseCommand_noCert = "wget --no-check-certificate -qO- ";

//stream utils
char* web_stream0=0;
char* web_stream1=0;
unsigned int web_streamBufferSize=0;
void web_setStreamSize(unsigned int size) {
	web_streamBufferSize = size;
	if(web_stream0) free(web_stream0);
	if(web_stream1) free(web_stream1);
	web_stream0 = 0;
	web_stream1 = 0;

	if(size > 0) {
		web_stream0 = malloc(web_streamBufferSize);
		web_stream1 = malloc(web_streamBufferSize);
		for(; size>0; size--) {
			web_stream0[size-1]=0;
			web_stream1[size-1]=0;
		}
	}
}
char web_collectData(FILE *fp) {
	char* tmp = web_stream0;
	web_stream0 = web_stream1;
	web_stream1 = tmp;
	
	char notAtEOF=1;
	unsigned int i;
	for(i=0; i<web_streamBufferSize; i++) {
		if(notAtEOF) {
			int c = fgetc(fp);
			if(c == EOF) {
				c = 0;
				notAtEOF = 0;
			}
			web_stream1[i] = c;
		} else {
			web_stream1[i]=0;
		}
	}
	return notAtEOF;
}
char web_getByteInStream(unsigned int index) {
	if(index < web_streamBufferSize) return web_stream0[index];
	else if(index < web_streamBufferSize*2) return web_stream1[index-web_streamBufferSize];
	return 0;
}
long web_indexOf(const char* text, unsigned int length, unsigned int start) {
	unsigned int i, j;
	for(i=start; i<(web_streamBufferSize*2)-length; i++) {
		char match = 1;
		for(j=0; j<length; j++) {
			if(text[j] != web_getByteInStream(i+j)) {
				match = 0;
				break;
			}
		}
		if(match) return i;
	}
	return -1;
}

//html decoding utils
char web_getNextCharDecoded();
unsigned int web_checkStringMatch(const char* text, unsigned int index) {
	unsigned int i;
	for(i=0; text[i]; i++) if(web_getByteInStream(index+i)!=text[i]) return 0;
	return i-1;
}
unsigned int web_checkTagMatch(const char* tag, unsigned int index) {
	unsigned int i;
	unsigned int length=0;
	if(web_getByteInStream(index) == '<') {
		length++;
		if(web_getByteInStream(index+length) == '/') length++;
		
		for(i=0; tag[i]; i++) if(web_getByteInStream(index+length+i)!=tag[i]) return 0;
		length+=i;
		
		unsigned char inQuote=0;
		for(i=0; i<index+length+(web_streamBufferSize*2); i++) {
			char c = web_getByteInStream(index+length+i);
			if(c == '"') {
				if(inQuote==0) inQuote=1;
				else inQuote=0;
			}
			if(c == '>' && inQuote == 0) {
				return length+i;
			}
		}
	}
	return 0;
}
char web_getDecValue(char hex) {
	if(hex>=48 && hex<=57) return hex-48;
	return 0;
}
char web_getHexValue(char hex) {
	if(hex>=48 && hex<=57) return hex-48;
	if(hex>=65 && hex<=70) return hex-55;
	if(hex>=97 && hex<=102) return hex-87;
	return 0;
}

//! Extracts data from a url.
void web_request(char* buffer, unsigned int bufferSize, const char* url, const char* afterFilter, const char* beforeFilter, unsigned int index, char noCert)
{
	unsigned int i, offset, urlSize, baseUrlSize, beforeSize, afterSize;
	if(afterFilter && afterFilter[0]==0) afterFilter = 0;
	if(beforeFilter && beforeFilter[0]==0) beforeFilter = 0;
	buffer[0] = 0;
	if(url) {
		
		//get string lengths
		if(noCert) for(baseUrlSize=0; web_baseCommand_noCert[baseUrlSize]; baseUrlSize++);
		else for(baseUrlSize=0; web_baseCommand[baseUrlSize]; baseUrlSize++);
		for(urlSize=0; url[urlSize]; urlSize++);
		for(beforeSize=0; beforeFilter && beforeFilter[beforeSize]; beforeSize++);
		for(afterSize=0; afterFilter && afterFilter[afterSize]; afterSize++);
		
		//construct command
		char* command = malloc(urlSize+baseUrlSize+1);
		if(noCert) for(i=0; web_baseCommand_noCert[i]; i++) command[i] = web_baseCommand_noCert[i];
		else for(i=0; web_baseCommand[i]; i++) command[i] = web_baseCommand[i];
		for(i=0; url[i]; i++) command[baseUrlSize+i] = url[i];
		command[urlSize+baseUrlSize] = 0;
		
		FILE *fp = popen(command, "r");
		if (fp != NULL) {
			unsigned int totalBufferSize = (bufferSize*4)+beforeSize+afterSize;
			web_setStreamSize(totalBufferSize);
			
			char newData = 1;
			while(newData) {
				newData = web_collectData(fp);
				
				//search for before and after filter text
				long beforeIndex = totalBufferSize*2;
				long afterIndex = totalBufferSize;
				long afterIndexStart = afterIndex;
				if(afterFilter) afterIndex = web_indexOf(afterFilter, afterSize, 0);
				if(afterIndex>-1) {
					afterIndexStart = afterIndex;
					afterIndex += afterSize;
					if(beforeFilter) beforeIndex = web_indexOf(beforeFilter, beforeSize, afterIndex);
				} else {
					if(beforeFilter) beforeIndex = web_indexOf(beforeFilter, beforeSize, 0);
				}
				
				if(beforeFilter && beforeIndex>-1 && !afterFilter) { 
					afterIndex = beforeIndex-bufferSize;
					if(afterIndex<0) afterIndex = 0;
				}
				
				//if we found our text, copy it into the buffer
				if(index==0 && beforeIndex>-1 && afterIndex>-1 && afterIndex<beforeIndex) {
					offset=0;
					for(i=0; (i+offset)<(bufferSize-1) && (i+offset)<(beforeIndex-afterIndex); i++) {
						buffer[i] = web_getNextCharDecoded(afterIndex+i, &offset);
					}
					buffer[i] = 0;
					break;
				}
				
				if(index > 0) {
					if(afterFilter) {
						if(afterIndex>-1 && afterIndexStart<totalBufferSize) index--;
					} else if(beforeFilter) {
						if(beforeIndex>-1 && beforeIndex<totalBufferSize) index--;
					}
				}
			}
	
			web_setStreamSize(0);
			pclose(fp);
		}
		
		free(command);
	}
}

//! Decodes HTML elements while fetching characters
char web_getNextCharDecoded(unsigned int index, unsigned int* offset) {
	unsigned int off = (*offset);
	unsigned int c = web_getByteInStream(off+index);
	
	//special entities (http://www.htmlhelp.com/reference/html40/entities/special.html)
	unsigned int length=0;
	if((length = web_checkStringMatch("&nbsp;", off+index)) > 0) c = ' ';
	else if((length = web_checkStringMatch("&apos;", off+index)) > 0) c = '\'';
	else if((length = web_checkStringMatch("&quot;", off+index)) > 0) c = '"';
	else if((length = web_checkStringMatch("&amp;", off+index)) > 0) c = '&';
	else if((length = web_checkStringMatch("&lt;", off+index)) > 0) c = '<';
	else if((length = web_checkStringMatch("&gt;", off+index)) > 0) c = '>';
	else if((length = web_checkStringMatch("&OElig;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&oelig;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&Scaron;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&scaron;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&Yuml;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&circ;", off+index)) > 0) c = '^';
	else if((length = web_checkStringMatch("&tilde;", off+index)) > 0) c = '~';
	else if((length = web_checkStringMatch("&ensp;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&emsp;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&thinsp;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&zwnj;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&zwj;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&lrm;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&rlm;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&ndash;", off+index)) > 0) c = '-';
	else if((length = web_checkStringMatch("&mdash;", off+index)) > 0) c = '-';
	else if((length = web_checkStringMatch("&lsquo;", off+index)) > 0) c = '\'';
	else if((length = web_checkStringMatch("&rsquo;", off+index)) > 0) c = '\'';
	else if((length = web_checkStringMatch("&sbquo;", off+index)) > 0) c = ',';
	else if((length = web_checkStringMatch("&ldquo;", off+index)) > 0) c = '"';
	else if((length = web_checkStringMatch("&rdquo;", off+index)) > 0) c = '"';
	else if((length = web_checkStringMatch("&bdquo;", off+index)) > 0) c = '"';
	else if((length = web_checkStringMatch("&dagger;", off+index)) > 0) c = '+';
	else if((length = web_checkStringMatch("&Dagger;", off+index)) > 0) c = '+';
	else if((length = web_checkStringMatch("&permil;", off+index)) > 0) c = '%';
	else if((length = web_checkStringMatch("&lsaquo;", off+index)) > 0) c = '<';
	else if((length = web_checkStringMatch("&rsaquo;", off+index)) > 0) c = '>';
	else if((length = web_checkStringMatch("&euro;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&cent;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&pound;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&yen;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&copy;", off+index)) > 0) c = 0;
	else if((length = web_checkStringMatch("&reg;", off+index)) > 0) c = 0;
	if(length > 0) {
		off+=length;
	}
	
	//decimal special entities (http://www.htmlhelp.com/reference/html40/entities/special.html)
	else if(web_getByteInStream(off+index)=='&' && web_getByteInStream(off+index+1)=='#') {
		c=0;
		off+=2;
		if(web_getByteInStream(off+index)!=';') {
			c=web_getDecValue(web_getByteInStream(off+index));
			off++;
			if(web_getByteInStream(off+index)!=';') {
				c=(c*10)+web_getDecValue(web_getByteInStream(off+index));
				off++;
				if(web_getByteInStream(off+index)!=';') {
					c=(c*10)+web_getDecValue(web_getByteInStream(off+index));
					off++;
					if(web_getByteInStream(off+index)!=';') {
						c=(c*10)+web_getDecValue(web_getByteInStream(off+index));
						off++;
						if(web_getByteInStream(off+index)!=';') {
							//more than 4 digits, not supported
							c=0;
							off++;
						}
					}
				}
			}
		}
		if(c>255) {
			if(c == 710) c = '^';
			else if(c == 732) c = '~';
			else if(c == 8211) c = '-';
			else if(c == 8212) c = '-';
			else if(c == 8216) c = '\'';
			else if(c == 8217) c = '\'';
			else if(c == 8218) c = ',';
			else if(c == 8220) c = '"';
			else if(c == 8221) c = '"';
			else if(c == 8222) c = '"';
			else if(c == 8224) c = '+';
			else if(c == 8225) c = '+';
			else if(c == 8240) c = '%';
			else if(c == 8249) c = '<';
			else if(c == 8250) c = '>';
			else c = 0;
		}
	}
	
	//hex special entities (http://www.htmlhelp.com/reference/html40/entities/special.html)
	else if(web_getByteInStream(off+index)=='&' && web_getByteInStream(off+index+1)=='#' && web_getByteInStream(off+index+2)=='x') {
		c=0;
		off+=3;
		if(web_getByteInStream(off+index)!=';') {
			c=web_getHexValue(web_getByteInStream(off+index));
			off++;
			if(web_getByteInStream(off+index)!=';') {
				c=(c*16)+web_getHexValue(web_getByteInStream(off+index));
				off++;
				if(web_getByteInStream(off+index)!=';') {
					c=(c*16)+web_getHexValue(web_getByteInStream(off+index));
					off++;
					if(web_getByteInStream(off+index)!=';') {
						c=(c*16)+web_getHexValue(web_getByteInStream(off+index));
						off++;
						if(web_getByteInStream(off+index)!=';') {
							//more than 4 digits, not supported
							c=0;
							off++;
						}
					}
				}
			}
		}
		if(c>255) {
			if(c == 0x2C6) c = '^';
			else if(c == 0x2DC) c = '~';
			else if(c == 0x2013) c = '-';
			else if(c == 0x2014) c = '-';
			else if(c == 0x2018) c = '\'';
			else if(c == 0x2019) c = '\'';
			else if(c == 0x201A) c = ',';
			else if(c == 0x201C) c = '"';
			else if(c == 0x201D) c = '"';
			else if(c == 0x201E) c = '"';
			else if(c == 0x2020) c = '+';
			else if(c == 0x2021) c = '+';
			else if(c == 0x2030) c = '%';
			else if(c == 0x2039) c = '<';
			else if(c == 0x203A) c = '>';
			else c = 0;
		}
	}
	
	//remove html tags (https://www.w3schools.com/tags/)
	else if(c == '<') {
		if((length = web_checkTagMatch("a", off+index)) > 0);
		else if((length = web_checkTagMatch("abbr", off+index)) > 0);
		else if((length = web_checkTagMatch("acronym", off+index)) > 0);
		else if((length = web_checkTagMatch("address", off+index)) > 0);
		else if((length = web_checkTagMatch("applet", off+index)) > 0);
		else if((length = web_checkTagMatch("area", off+index)) > 0);
		else if((length = web_checkTagMatch("article", off+index)) > 0);
		else if((length = web_checkTagMatch("aside", off+index)) > 0);
		else if((length = web_checkTagMatch("audio", off+index)) > 0);
		else if((length = web_checkTagMatch("b", off+index)) > 0);
		else if((length = web_checkTagMatch("base", off+index)) > 0);
		else if((length = web_checkTagMatch("basefont", off+index)) > 0);
		else if((length = web_checkTagMatch("bdi", off+index)) > 0);
		else if((length = web_checkTagMatch("bdo", off+index)) > 0);
		else if((length = web_checkTagMatch("big", off+index)) > 0);
		else if((length = web_checkTagMatch("blockquote", off+index)) > 0);
		else if((length = web_checkTagMatch("body", off+index)) > 0);
		else if((length = web_checkTagMatch("br", off+index)) > 0);
		else if((length = web_checkTagMatch("button", off+index)) > 0);
		else if((length = web_checkTagMatch("canvas", off+index)) > 0);
		else if((length = web_checkTagMatch("caption", off+index)) > 0);
		else if((length = web_checkTagMatch("center", off+index)) > 0);
		else if((length = web_checkTagMatch("cite", off+index)) > 0);
		else if((length = web_checkTagMatch("code", off+index)) > 0);
		else if((length = web_checkTagMatch("col", off+index)) > 0);
		else if((length = web_checkTagMatch("colgroup", off+index)) > 0);
		else if((length = web_checkTagMatch("datalist", off+index)) > 0);
		else if((length = web_checkTagMatch("dd", off+index)) > 0);
		else if((length = web_checkTagMatch("del", off+index)) > 0);
		else if((length = web_checkTagMatch("details", off+index)) > 0);
		else if((length = web_checkTagMatch("dfn", off+index)) > 0);
		else if((length = web_checkTagMatch("dialog", off+index)) > 0);
		else if((length = web_checkTagMatch("dir", off+index)) > 0);
		else if((length = web_checkTagMatch("div", off+index)) > 0);
		else if((length = web_checkTagMatch("dl", off+index)) > 0);
		else if((length = web_checkTagMatch("dt", off+index)) > 0);
		else if((length = web_checkTagMatch("em", off+index)) > 0);
		else if((length = web_checkTagMatch("embed", off+index)) > 0);
		else if((length = web_checkTagMatch("fieldset", off+index)) > 0);
		else if((length = web_checkTagMatch("figcaption", off+index)) > 0);
		else if((length = web_checkTagMatch("figure", off+index)) > 0);
		else if((length = web_checkTagMatch("font", off+index)) > 0);
		else if((length = web_checkTagMatch("footer", off+index)) > 0);
		else if((length = web_checkTagMatch("form", off+index)) > 0);
		else if((length = web_checkTagMatch("frame", off+index)) > 0);
		else if((length = web_checkTagMatch("frameset", off+index)) > 0);
		else if((length = web_checkTagMatch("h1", off+index)) > 0);
		else if((length = web_checkTagMatch("h2", off+index)) > 0);
		else if((length = web_checkTagMatch("h3", off+index)) > 0);
		else if((length = web_checkTagMatch("h4", off+index)) > 0);
		else if((length = web_checkTagMatch("h5", off+index)) > 0);
		else if((length = web_checkTagMatch("h6", off+index)) > 0);
		else if((length = web_checkTagMatch("head", off+index)) > 0);
		else if((length = web_checkTagMatch("header", off+index)) > 0);
		else if((length = web_checkTagMatch("hr", off+index)) > 0);
		else if((length = web_checkTagMatch("html", off+index)) > 0);
		else if((length = web_checkTagMatch("i", off+index)) > 0);
		else if((length = web_checkTagMatch("iframe", off+index)) > 0);
		else if((length = web_checkTagMatch("img", off+index)) > 0);
		else if((length = web_checkTagMatch("input", off+index)) > 0);
		else if((length = web_checkTagMatch("ins", off+index)) > 0);
		else if((length = web_checkTagMatch("kbd", off+index)) > 0);
		else if((length = web_checkTagMatch("label", off+index)) > 0);
		else if((length = web_checkTagMatch("legend", off+index)) > 0);
		else if((length = web_checkTagMatch("li", off+index)) > 0);
		else if((length = web_checkTagMatch("link", off+index)) > 0);
		else if((length = web_checkTagMatch("main", off+index)) > 0);
		else if((length = web_checkTagMatch("map", off+index)) > 0);
		else if((length = web_checkTagMatch("mark", off+index)) > 0);
		else if((length = web_checkTagMatch("menu", off+index)) > 0);
		else if((length = web_checkTagMatch("menuitem", off+index)) > 0);
		else if((length = web_checkTagMatch("meta", off+index)) > 0);
		else if((length = web_checkTagMatch("meter", off+index)) > 0);
		else if((length = web_checkTagMatch("nav", off+index)) > 0);
		else if((length = web_checkTagMatch("noframes", off+index)) > 0);
		else if((length = web_checkTagMatch("noscript", off+index)) > 0);
		else if((length = web_checkTagMatch("object", off+index)) > 0);
		else if((length = web_checkTagMatch("ol", off+index)) > 0);
		else if((length = web_checkTagMatch("optgroup", off+index)) > 0);
		else if((length = web_checkTagMatch("option", off+index)) > 0);
		else if((length = web_checkTagMatch("output", off+index)) > 0);
		else if((length = web_checkTagMatch("p", off+index)) > 0);
		else if((length = web_checkTagMatch("param", off+index)) > 0);
		else if((length = web_checkTagMatch("picture", off+index)) > 0);
		else if((length = web_checkTagMatch("pre", off+index)) > 0);
		else if((length = web_checkTagMatch("progress", off+index)) > 0);
		else if((length = web_checkTagMatch("q", off+index)) > 0);
		else if((length = web_checkTagMatch("rp", off+index)) > 0);
		else if((length = web_checkTagMatch("rt", off+index)) > 0);
		else if((length = web_checkTagMatch("ruby", off+index)) > 0);
		else if((length = web_checkTagMatch("s", off+index)) > 0);
		else if((length = web_checkTagMatch("samp", off+index)) > 0);
		else if((length = web_checkTagMatch("script", off+index)) > 0);
		else if((length = web_checkTagMatch("section", off+index)) > 0);
		else if((length = web_checkTagMatch("select", off+index)) > 0);
		else if((length = web_checkTagMatch("small", off+index)) > 0);
		else if((length = web_checkTagMatch("source", off+index)) > 0);
		else if((length = web_checkTagMatch("span", off+index)) > 0);
		else if((length = web_checkTagMatch("strike", off+index)) > 0);
		else if((length = web_checkTagMatch("strong", off+index)) > 0);
		else if((length = web_checkTagMatch("style", off+index)) > 0);
		else if((length = web_checkTagMatch("sub", off+index)) > 0);
		else if((length = web_checkTagMatch("summary", off+index)) > 0);
		else if((length = web_checkTagMatch("sup", off+index)) > 0);
		else if((length = web_checkTagMatch("table", off+index)) > 0);
		else if((length = web_checkTagMatch("tbody", off+index)) > 0);
		else if((length = web_checkTagMatch("td", off+index)) > 0);
		else if((length = web_checkTagMatch("textarea", off+index)) > 0);
		else if((length = web_checkTagMatch("tfoot", off+index)) > 0);
		else if((length = web_checkTagMatch("th", off+index)) > 0);
		else if((length = web_checkTagMatch("thead", off+index)) > 0);
		else if((length = web_checkTagMatch("time", off+index)) > 0);
		else if((length = web_checkTagMatch("title", off+index)) > 0);
		else if((length = web_checkTagMatch("tr", off+index)) > 0);
		else if((length = web_checkTagMatch("track", off+index)) > 0);
		else if((length = web_checkTagMatch("tt", off+index)) > 0);
		else if((length = web_checkTagMatch("u", off+index)) > 0);
		else if((length = web_checkTagMatch("ul", off+index)) > 0);
		else if((length = web_checkTagMatch("var", off+index)) > 0);
		else if((length = web_checkTagMatch("video", off+index)) > 0);
		else if((length = web_checkTagMatch("wbr", off+index)) > 0);	
		if(length > 0) {
			*offset = off+length+1;
			return web_getNextCharDecoded(index, offset);
		}
	}
	
	*offset = off;
	return c;
}
