PLATFORM = X11
#PLATFORM = WIN32
#PLATFORM = NO_GRAPHICS

INC = ../easygl
EXE = router
HDR = $(INC)/graphics.h $(INC)/easygl_constants.h channel.h network.h graph_painter.h
#FLAGS = -g -w -D$(PLATFORM)
FLAGS = -g -Wall -std=c++11 -D$(PLATFORM)

# Need to tell the linker to link to the X11 libraries.
# WIN32 automatically links to the win32 API libraries (no need for flags)
ifeq ($(PLATFORM),X11)
   GRAPHICS_LIBS = -lX11
endif

$(EXE): router.o channel.o network.o graph_painter.o $(INC)/graphics.o
	
	g++ $(FLAGS) $(INC)/graphics.o router.o channel.o network.o graph_painter.o $(GRAPHICS_LIBS) -o $(EXE)

router.o: router.cpp $(HDR)
	g++ -c $(FLAGS) -I $(INC) router.cpp

graph_painter.o: graph_painter.cpp $(HDR)
	g++ -c $(FLAGS) -I $(INC) graph_painter.cpp

network.o: network.cpp $(HDR)
	g++ -c $(FLAGS) -I $(INC) network.cpp

channel.o: channel.cpp $(HDR)
	g++ -c $(FLAGS) -I $(INC) channel.cpp

clean:
	rm $(EXE) *.o