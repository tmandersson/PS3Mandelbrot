#-- PS3/PSL1GHT stuff -----------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/ppu_rules

TITLE		:=	Mandelbrot plotter
APPID		:=	MANDELBRO
CONTENTID	:=	TMAnd0-$(APPID)_00-0000000000000000

PS3_CC := $(CXX) 
PS3_CFLAGS = -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
export LD	:=	$(CXX)
LDFLAGS		=	$(MACHDEP) -Wl,-Map,$(notdir $@).map
export BUILDDIR	:=	$(CURDIR)/ps3_build
#-- END PS3/PSL1GHT stuff -----------------------------------------

#-- C/C++ linux sdl stuff -----------------------------------------
CC = g++
CPPFLAGS = -Wall -O3
#-- End C/C++ linux sdl stuff -----------------------------------------

all : sdl_mandel
ps3 : mandel.self mandel.pkg

mandel.self : mandel.elf

mandel.elf : ps3_main.o
	$(PS3_CC) $(PS3_CFLAGS) -o mandel.elf ps3_main.o
	
ps3_main.o : ps3_main.cpp
	$(PS3_CC) $(PS3_CFLAGS) -c ps3_main.cpp

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
