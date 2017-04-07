# Example Makefile script
# Purpose: Demonstrate usage of pkg-config with MRPT libraries
# By: Jose Luis Blanco, 2010.
#
# ========================= *IMPORTANT* ================================
# For this method to work MRPT must be installed in your
# system in a path accesible to pkg-config. To check if pkg-config
# sees MRPT config files, execute:
# pkg-config --list-all | grep mrpt
# If no package appears, MRPT is not installed or something else is wrong.
# ======================================================================
#
 
# Set up basic variables:
CC = g++
CFLAGS = -c -Wall /usr/local/lib
LDFLAGS = -lpthread /usr/local/lib

# List of sources:
INCLUDES= 
SOURCES = moveDetect.cpp timer.h cameraInterface.h utils.h rpiPWM1.h rpiPWM1.cpp
OBJECTS = $(SOURCES:.cpp=.o)
 
# Name of executable target:
TARGET = moveDetect
 
# MRPT specific flags:
# Here we invoke "pkg-config" passing it as argument the list of the
# MRPT libraries needed by our program (see available libs
# with "pkg-config --list-all | grep mrpt").
#
CFLAGS += -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util
LDFLAGS += -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util
 
all: $(SOURCES) $(TARGET)
 
$(TARGET): $(OBJECTS)
	$(CC) --std=c++14 -O3 $(OBJECTS) -o $@ $(LDFLAGS) 
 
.cpp.o:
	$(CC) --std=c++14 -O3 $(CFLAGS) $(INCLUDES) $< -o $@
 
clean:
	rm *.o $(TARGET)
