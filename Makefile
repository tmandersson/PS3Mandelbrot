#-- PS3/PSL1GHT stuff -----------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/ppu_rules

TITLE		:=	Mandelbrot plotter
APPID		:=	MANDELBRO
CONTENTID	:=	TMAnd0-$(APPID)_00-0000000000000000

PS3_CC := $(CXX)

# override MACHDEPT from ppu_rules
# and disable some optimizations to allow profiling
# MACHDEP = 
PS3_CFLAGS = -g -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
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
					$(LIBPSL1GHT_INC) -I$(CURDIR)/include \
					-I$(PS3DEV)/ppu/lib/gcc/powerpc64-ps3-elf/4.5.2/include \
					-I$(CURDIR)
#-- END PS3/PSL1GHT stuff -----------------------------------------

#-- C/C++ linux sdl stuff -----------------------------------------
CC = g++
CPPFLAGS = -Wall -g -O3 -I$(CURDIR)/include
#-- End C/C++ linux sdl stuff -----------------------------------------

vpath %.cpp src src/core src/ps3 src/linux src/spu_test
vpath %.c src src/core/ src/ps3 src/linux src/spu_test
vpath %.h include include/core include/ps3 include/linux

.PHONY: spu.bin

all: spu.bin.o sdl_mandel ps3
ps3: spu.bin.o mandel.self mandel.pkg spu_test.self spu_test.pkg
run:
	ps3load mandel.self
run_test:
	ps3load spu_test.self
	
mandel.self: mandel.elf
mandel.elf: ps3_main.pso rsxutil.pso palette.pso rsxplotter.pso mandel.pso
spu_test.self: spu_test.elf
spu_test.elf: spu_test_main.pso spu.bin.o

%.elf:
	$(PS3_CC) $(PS3_CFLAGS) -o $@ $^ $(LIBS)
%.pso: %.cpp %.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<
%.pso: %.cpp
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<
%.pso: %.c %.h
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<
%.pso: %.c
	$(PS3_CC) $(PS3_CFLAGS) -o $@ -c $<
	
spu.bin:
	@$(MAKE) -C src/spu
	
%.bin.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)
	
sdl_mandel: sdl_main.lo mandel.lo sdlplotter.lo palette.lo
	$(CC) $(CPPFLAGS) -o $@ $^ `sdl-config --libs` -pg

%.lo: %.cpp %.h
	$(CC) $(CPPFLAGS) -o $@ -c $< -pg
%.lo: %.cpp
	$(CC) $(CPPFLAGS) -o $@ -c $< -pg
%.lo: %.c %.h
	$(CC) $(CPPFLAGS) -o $@ -c $< -pg
%.lo: %.c
	$(CC) $(CPPFLAGS) -o $@ -c $< -pg

clean: clean-all
clean-all:
	  -rm -f mandel sdl_mandel *.o *.lo *.pso *.elf *.self *.pkg *.bin spu_bin.h
	  @$(MAKE) -C src/spu clean 
