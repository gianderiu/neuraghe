#	Author: Daniele Palossi			
#	email: dpalossi@iis.ee.ethz.ch	


SRC = ./src/*.cpp
OUT = ./bin/client

CC            = gcc
CXX           = g++

################################ Compiler Flags ################################
DEFINES       = -DQT_DEPRECATED_WARNINGS -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -std=gnu++11 -Wall -W -D_REENTRANT -fPIC $(DEFINES)

############################# OpenCV Home Directory ############################
OPENCV_HOME = /home/nur/OpenCV/

################################ Include Dir ###################################
INCPATH += -I$(OPENCV_HOME)/include/opencv-3.4.0
INCPATH += -I$(OPENCV_HOME)/include

################################## Library Dir #################################
LDIR += -L$(OPENCV_HOME)/build/lib
LDIR += -L/home/nur/OpenCV/3rdparty/libjpeg

################################# Library Flags ################################
LDFLAGS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lpthread -ljpeg

#################################### Targets ###################################
clean:
	rm -rf *.o $(OUT) ./bin/*.bin

build:
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCPATH) $(SRC) -o $(OUT) $(LDIR) $(LDFLAGS)

all: clean build
