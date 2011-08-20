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

	Palette palette = Palette();
	RSXPlotter plotter = RSXPlotter(&buffers[current_buffer], palette);
	Mandel mand = Mandel(ITERATIONS, width, height, plotter);

	waitFlip();
	mand.zoom(-2.625531-(0.002875*0), 1.054469-(0.002875*0), -0.715000-(0.002888889*0), 1.365000-(0.002888889*0));
	flip(context, buffers[current_buffer].id);

	while(1) {
		// Change to other buffer.
		if (current_buffer < (MAX_BUFFERS - 1))
			current_buffer++;
		else
			current_buffer = 0;
		plotter.setSurface(&buffers[current_buffer]);

		// zoom and draw
		waitFlip();
		mand.zoom_coord(width*0.01, height*0.01, width-width*0.01, height-height*0.01);
		flip(context, buffers[current_buffer].id);

		// check for user input for exit
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

