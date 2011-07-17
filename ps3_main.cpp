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
#include "rsxutil.h"

#define MAX_BUFFERS 2

void drawFrame(rsxBuffer*, long);
void plot(rsxBuffer*, int, int, int, int, int);

int main(s32 argc, const char* argv[])
{
  gcmContextData *context;
  void *host_addr = NULL;
  rsxBuffer buffers[MAX_BUFFERS];
  int currentBuffer = 0;
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

  long frame = 0; // To keep track of how many frames we have rendered.
  waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
  drawFrame(&buffers[currentBuffer], frame++); // Draw into the unused buffer
  flip(context, buffers[currentBuffer].id); // Flip buffer onto screen

  currentBuffer++;
  if (currentBuffer >= MAX_BUFFERS)
    currentBuffer = 0;

  printf("Picture has been drawn.\n");
	
  // Ok, everything is setup. Now for the main loop.
  while(1){
    // Check the pads.
    ioPadGetInfo(&padinfo);
    for(i=0; i<MAX_PADS; i++){
      if(padinfo.status[i]){
	ioPadGetData(i, &paddata);
				
	if(paddata.BTN_START){
	  goto end;
	}
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

void drawFrame(rsxBuffer *buffer, long frame) {
  s32 i, j;
  for(i = 0; i < buffer->height; i++) {
    s32 color = (i / (buffer->height * 1.0) * 256);
    // This should make a nice black to green graident
    color = (color << 8) | ((frame % 255) << 16);
    for(j = 0; j < buffer->width; j++)
      plot(buffer, j, i, 0xFF, 0x00, 0xFF);
  }
}

void plot(rsxBuffer *buffer, int x, int y, int r, int g, int b) {
	u32 color = 0x00000000;
	color += (u32) (r << 16);
	color += (u32) (g << 8);
	color += b;
	printf("Color %x32\n", color);
	buffer->ptr[y * buffer->width + x] = color;
}
