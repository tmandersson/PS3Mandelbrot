# C/C++ compiler
CC = g++

CPPFLAGS = -Wall -O3


all : mandel

mandel : main.o mandel.o sdlplotter.o
	$(CC) $(CPPFLAGS) -o mandel main.o mandel.o sdlplotter.o `sdl-config --libs`

main.o : main.cpp
	$(CC) $(CPPFLAGS) -c main.cpp 
mandel.o : mandel.cpp
	 $(CC) $(CPPFLAGS) -c mandel.cpp
sdlplotter.o : sdlplotter.cpp
	   $(CC) $(CPPFLAGS) -c sdlplotter.cpp
	   
clean : clean-all

clean-all:
	  -rm -f mandel *.o
