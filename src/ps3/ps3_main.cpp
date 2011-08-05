#include <ppu-lv2.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <sysutil/video.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>

#include <io/pad.h>
#include "ps3/rsxutil.h"

#include "core/palette.h"
#include "ps3/rsxplotter.h"

#define MAX_BUFFERS 2

const unsigned int ITERATIONS = 50;

void drawFrame(rsxBuffer*);
void plot(rsxBuffer*, int, int, u32);

int main(int argc,const char *argv[])
{
	gcmContextData *context;
	void *host_addr = NULL;
	rsxBuffer buffers[MAX_BUFFERS];
	int current_buffer = 0;
	padInfo padinfo;
	padData paddata;
	u16 width;
	u16 height;
	int i;

	printf("Running Mandel video test!\n");

	/* Allocate a 1Mb buffer, aligned to a 1Mb boundary
	* to be our shared IO memory with the RSX. */
	host_addr = memalign (1024*1024, HOST_SIZE);
	context = initScreen (host_addr, HOST_SIZE);
	ioPadInit(7);

	getResolution(&width, &height);
	for (i = 0; i < MAX_BUFFERS; i++)
		makeBuffer( &buffers[i], width, height, i);

	printf("Resolution: %i x %i\n", width, height);
	printf("Video init done.\n");

	flip(context, MAX_BUFFERS - 1);

	printf("First flip done.\n");

	Palette palette = Palette();
	RSXPlotter plotter = RSXPlotter(&buffers[current_buffer], palette);
	Mandel mand = Mandel(ITERATIONS, width, height, plotter);

	// 1
	waitFlip();
	mand.zoom(-3.0, 1.6, -1.3, 1.3);
	flip(context, buffers[current_buffer].id);
	if (current_buffer < MAX_BUFFERS)
		current_buffer++;
	else
		current_buffer = 0;
	plotter.setSurface(&buffers[current_buffer]);

	// 2
	waitFlip();
	mand.zoom(-2.0, 1.1, -0.9, 0.9);
	flip(context, buffers[current_buffer].id);

	printf("Pictures has been drawn.\n");
	
	while(1) {
		ioPadGetInfo(&padinfo);
		for(i=0; i<MAX_PADS; i++) {
			if(padinfo.status[i]) {
				ioPadGetData(i, &paddata);
				
				if(paddata.BTN_START)
					goto end;
			}
		}
	}
	
	end:

	gcmSetWaitFlip(context);
	for (i = 0; i < MAX_BUFFERS; i++)
		rsxFree(buffers[i].ptr);

	rsxFinish(context, 1);
	free(host_addr);
	ioPadEnd();

	printf("Exiting!\n");

	return 0;
}

u32 GetARGB32(int r, int g, int b)
{
	u32 result = 0x00000000;
	result += (u32)((r << 16));
	result += (u32)((g << 8));
	result += b;
	return result;
}

void drawFrame(rsxBuffer *buffer) {
	s32 i, j;
	u32 color = GetARGB32(0xFF, 0x00, 0xFF);
	printf("Chosen color: 0x%lx\n", (long unsigned int) color);
	for(i = 0; i < buffer->height; i++) {
		for(j = 0; j < buffer->width; j++)
		  plot(buffer, j, i, color);
	}
}

void plot(rsxBuffer *buffer, int x, int y, u32 color) {
	buffer->ptr[y * buffer->width + x] = color;
}
