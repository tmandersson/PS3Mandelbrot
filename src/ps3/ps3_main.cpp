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
	Mandel mand = Mandel(width, height, plotter);

	waitFlip();

	// -0.743643887037158704752191506114774, 0.131825904205311970493132056385139
	double start_real = -0.743643887037158704752191506114774;
	double start_imag = 0.131825904205311970493132056385139;
	double x_aspect = 16;
	double y_aspect = 9;
	double offset_real = x_aspect * 0.1;
	double offset_imag = y_aspect * 0.1;
	mand.zoom(start_real-offset_real, start_real+offset_real, start_imag-offset_imag, start_imag+offset_imag);

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
		mand.zoom_coord(x_aspect, y_aspect, width-x_aspect, height-y_aspect);
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

