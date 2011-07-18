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

#include "palette.h"

#define MAX_BUFFERS 2

void drawFrame(rsxBuffer*);
void plot(rsxBuffer*, int, int, u32);

int main(int argc,const char *argv[])
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

  Palette palette = Palette();
  waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
  drawFrame(&buffers[currentBuffer]); // Draw into the unused buffer
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
