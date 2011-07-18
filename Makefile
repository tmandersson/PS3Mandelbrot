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
					$(LIBPSL1GHT_INC) \
					-I$(CURDIR)/$(BUILD)
#-- END PS3/PSL1GHT stuff -----------------------------------------

#-- C/C++ linux sdl stuff -----------------------------------------
CC = g++
CPPFLAGS = -Wall -O3
#-- End C/C++ linux sdl stuff -----------------------------------------

all : sdl_mandel ps3
ps3 : mandel.self mandel.pkg

run:
	ps3load mandel.self


mandel.self : mandel.elf

mandel.elf : ps3_main.o rsxutil.o ps3_palette.o
	$(PS3_CC) $(PS3_CFLAGS) -o mandel.elf ps3_main.o rsxutil.o ps3_palette.o $(LIBS)
	
ps3_main.o : ps3_main.cpp
	$(PS3_CC) $(PS3_CFLAGS) -c ps3_main.cpp
	
rsxutil.o : rsxutil.cpp
	$(PS3_CC) $(PS3_CFLAGS) -c rsxutil.cpp

ps3_palette.o : palette.cpp
	$(PS3_CC) $(PS3_CPPFLAGS) -o ps3_palette.o -c palette.cpp

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
