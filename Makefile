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

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lrsx -lgcm_sys -lsysutil -lio -llv2

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(LIBPSL1GHT_INC) -I$(CURDIR)/include
#-- END PS3/PSL1GHT stuff -----------------------------------------

#-- C/C++ linux sdl stuff -----------------------------------------
CC = g++
CPPFLAGS = -Wall -O3 -I$(CURDIR)/include
#-- End C/C++ linux sdl stuff -----------------------------------------

vpath %.cpp src src/core src/ps3 src/linux
vpath %.c src src/core/ src/ps3 src/linux
vpath %.h include include/core include/ps3 include/linux

all : sdl_mandel ps3
ps3 : mandel.self mandel.pkg

run:
	ps3load mandel.self


mandel.self : mandel.elf

mandel.elf : ps3_main.o rsxutil.o ps3_palette.o rsxplotter.o ps3_mandel.o
	$(PS3_CC) $(PS3_CFLAGS) -o $@ $^ $(LIBS)
	
ps3_main.o : ps3_main.cpp ps3_main.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<
	
rsxutil.o : rsxutil.cpp rsxutil.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<

ps3_palette.o : palette.cpp palette.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<

rsxplotter.o : rsxplotter.cpp rsxplotter.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<

ps3_mandel.o : mandel.cpp mandel.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<

sdl_mandel : sdl_main.o mandel.o sdlplotter.o palette.o
	$(CC) $(CPPFLAGS) -o $@ $^ `sdl-config --libs`

sdl_main.o : sdl_main.cpp sdl_main.h
	$(CC) $(CPPFLAGS) -c $<
mandel.o : mandel.cpp mandel.h
	$(CC) $(CPPFLAGS) -c $<
sdlplotter.o : sdlplotter.cpp sdlplotter.h
	$(CC) $(CPPFLAGS) -c $<
palette.o : palette.cpp palette.h
	$(CC) $(CPPFLAGS) -c $<
	   
clean : clean-all

clean-all:
	  -rm -f mandel sdl_mandel *.o *.elf *.self *.pkg
