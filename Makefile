# C/C++ compiler
CC = g++

CPPFLAGS = -Wall -O3


all : sdl_mandel

mandel.elf : ps3_main.o
	$(CC) $(CPPFLAGS) -o mandel.elf ps3_main.o
	
ps3_main.o : ps3_main.cpp
	$(CC) $(CPPFLAGS) -c ps3_main.cpp

sdl_mandel : sdl_main.o mandel.o sdlplotter.o palette.o
	$(CC) $(CPPFLAGS) -o sdl_mandel sdl_main.o mandel.o sdlplotter.o palette.o `sdl-config --libs`

sdl_main.o : sdl_main.cpp
	$(CC) $(CPPFLAGS) -c sdl_main.cpp 
mandel.o : mandel.cpp
	 $(CC) $(CPPFLAGS) -c mandel.cpp
sdlplotter.o : sdlplotter.cpp
	$(CC) $(CPPFLAGS) -c sdlplotter.cpp
palette.o : palette.cpp
	$(CC) $(CPPFLAGS) -c palette.cpp
	   
clean : clean-all

clean-all:
	  -rm -f sdl_mandel *.o *.elf *.self *.pkg
