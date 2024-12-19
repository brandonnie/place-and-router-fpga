# Pick a platform below.  X11 for Linux/Mac/Unix, and WIN32 for windows.
# In either X11 or WIN32, Postscript is also simultaneously available.
# You can also pick NO_GRAPHICS, which will allow your code to compile without
# change on any platform, but no graphics will display (all calls are stubbed
# out).
# 
# Compiling to support X11 requires the X11 development libraries. On Ubuntu, use
# 'sudo apt-get install libx11-dev' to install.
# Please look online for information on how to install X11 on other Linux distributions.

# Please note that a Mac can run the X11 graphics if the X11 library is installed.

PLATFORM = X11
#PLATFORM = WIN32
#PLATFORM = NO_GRAPHICS

HDR = graphics.h easygl_constants.h
SRC = graphics.cpp example.cpp
PARTITION_HDR = Netlist.h
PARTITION_SRC = partitioner.cpp Netlist.cpp
EXE = partitioner
BACKUP_FILENAME=`date "+backup-%Y%m%d-%H%M.zip"`
FLAGS = -g -O3 -std=c++11 -Wall -D$(PLATFORM)

# Need to tell the linker to link to the X11 libraries.
# WIN32 automatically links to the win32 API libraries (no need for flags)
ifeq ($(PLATFORM),X11)
   GRAPHICS_LIBS = -lX11
endif

partitioner: graphics.o $(PARTITION_SRC) $(PARTITION_HDR)
	g++ $(FLAGS) $(PARTITION_SRC) graphics.o $(GRAPHICS_LIBS) -o partitioner

example: graphics.o example.o
	g++ $(FLAGS) graphics.o example.o $(GRAPHICS_LIBS) -o example

graphics.o: graphics.cpp $(HDR)
	g++ -c $(FLAGS) graphics.cpp

example.o: example.cpp $(HDR)
	g++ -c $(FLAGS) example.cpp

backup:
# Back up the source, makefile and Visual Studio project & solution. 
	echo Backing up your files into ${BACKUP_FILENAME}
	zip ${BACKUP_FILENAME} $(SRC) $(HDR) makefile easygl.sln easygl.vcxproj

clean:
	rm $(EXE) *.o example
