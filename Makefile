#-- PS3/PSL1GHT stuff -----------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/ppu_rules

TITLE		:=	Mandelbrot plotter
APPID		:=	MANDELBRO
CONTENTID	:=	TMAnd0-$(APPID)_00-0000000000000000

CC := $(CXX)

# override MACHDEPT from ppu_rules
# and disable some optimizations to allow profiling
# MACHDEP = 
CFLAGS = -g -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
export LD	:=	$(CXX)
LDFLAGS		=	$(MACHDEP) -Wl,-Map,$(notdir $@).map
export BUILDDIR	:=	$(CURDIR)/build

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

vpath %.cpp src src/core src/ps3 src/spu_test
vpath %.c src src/core/ src/ps3 src/spu_test
vpath %.h include include/core include/ps3

.PHONY: spu.bin

all: spu.bin.o mandel.self mandel.pkg spu_test.self spu_test.pkg
run:
	ps3load mandel.self
run_test:
	ps3load spu_test.self
	
mandel.self: mandel.elf
mandel.elf: main.o rsxutil.o palette.o rsxplotter.o mandel.o
spu_test.self: spu_test.elf
spu_test.elf: spu_test_main.o rsxutil.o rsxplotter.o palette.o spu.bin.o

%.elf:
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
%.o: %.cpp %.h
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
spu.bin:
	@$(MAKE) -C src/spu
	
%.bin.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)
	
clean: clean-all
clean-all:
	  -rm -rf *.o *.elf *.self *.pkg *.bin spu_bin.h build
	  @$(MAKE) -C src/spu clean 
