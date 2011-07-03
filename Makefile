# Makefile
# Här skriver du sökvägen till qt om det inte funkar.
QT_DIR = $(QTDIR)
QT_LIB = /usr/qt/3/lib/libqt.so
QT_INCLUDE = /usr/qt/3/include

# Här skriver du vilken c++-kompilator du använder.
CC = g++

CPPFLAGS = -Wall -O3 -march=athlon-tbird


all : mandel conmandel tstcomplex

mandel : main.o qtmandel.o mandel.o
	$(CC) $(CPPFLAGS) -o mandel main.o qtmandel.o mandel.o $(QT_LIB)

#mandel : main.o sdlmandel.o 
#	$(CC) $(CPPFLAGS) -o mandel main.o sdlmandel.o mandel.o `sdl-config --libs`

conmandel : conmandel.o mandel.o
	   $(CC) $(CPPFLAGS) -o conmandel conmandel.o mandel.o
tstcomplex : tstcomplex.o
	    $(CC) $(CPPFLAGS) -o tstcomplex tstcomplex.o   
main.o : main.cpp
	$(CC) $(CPPFLAGS) -I $(QT_INCLUDE) -c main.cpp 
mandel.o : mandel.cpp
	 $(CC) $(CPPFLAGS) -c mandel.cpp
sdlmandel.o : sdlmandel.cpp mandel.o
	   $(CC) $(CPPFLAGS) -c sdlmandel.cpp
qtmandel.o : qtmandel.cpp
	$(CC) $(CPPFLAGS) -I $(QT_INCLUDE) -c qtmandel.cpp 
conmandel.o : conmandel.cpp
	     $(CC) $(CPPFLAGS) -c conmandel.cpp
tstcomplex.o : tstcomplex.cpp
	      $(CC) $(CPPFLAGS) -c tstcomplex.cpp

clean : clean-all

clean-all:
	  -rm -f mandel conmandel tstcomplex *.o
