# C/C++ compiler
CC = g++

CPPFLAGS = -Wall -O3 -march=athlon-tbird


all : mandel

mandel : main.o mandel.o sdlmandel.o
	$(CC) $(CPPFLAGS) -o mandel main.o mandel.o sdlmandel.o `sdl-config --libs`

main.o : main.cpp
	$(CC) $(CPPFLAGS) -c main.cpp 
mandel.o : mandel.cpp
	 $(CC) $(CPPFLAGS) -c mandel.cpp
sdlmandel.o : sdlmandel.cpp mandel.o
	   $(CC) $(CPPFLAGS) -c sdlmandel.cpp

clean : clean-all

clean-all:
	  -rm -f mandel *.o
