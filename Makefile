# Target Name
TARGET=PixelClock

# Directories
BUILDDIR=build
SOURCEDIR=source
INCDIR=inc

# Objects to Build
OBJECTS=$(BUILDDIR)/main.o $(BUILDDIR)/render.o $(BUILDDIR)/images.o $(BUILDDIR)/config.o $(BUILDDIR)/web.o

# Libraries to Include
LIBRARIES=-lwiringPi -pthread

# Compiler
CC=gcc
CXX=g++

# Flags
CFLAGS=
CXXFLAGS=$(CFLAGS)

#===============================================================================

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBRARIES)
	

$(BUILDDIR)/%.o : $(SOURCEDIR)/%.c
	$(CC) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

$(shell mkdir -p $(BUILDDIR))

clean:
	rm -f $(TARGET)
	rm -f $(BUILDDIR)/*.o

.PHONY: FORCE
