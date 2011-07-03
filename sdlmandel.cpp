// sdlmandel.cpp
#include <iostream>
#include "sdlmandel.h"
#include <SDL/SDL.h>
using std::cerr;

SDLMandel::SDLMandel(unsigned int iterations, int w, int h)
  : Mandel(iterations, w, h)
{
  // Initialize defaults, Video
  if ((SDL_Init(SDL_Init(SDL_INIT_VIDEO))==-1)) {
    cerr << "Could not initialize SDL: " << SDL_GetError() << ".\n";
    exit(1);
  }
	
  // Initialize the display
  // requesting a hardware surface
  pScreen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE);
  if ( pScreen == NULL ) {
    cerr << "Couldn't set " << w << "x" << h << "x32 video mode: " 
	 << SDL_GetError() << ".\n";
    exit(1);
  }
  zoom_start();
}

SDLMandel::~SDLMandel()
{
  SDL_Quit();
}

void SDLMandel::plot(int x, int y, int color)
{
  if (x == 0 && y == 0) {
    // lock the screen
    if (SDL_LockSurface(pScreen) != 0) {
      cerr << "Can't get access to screen!!\n";
      SDL_Quit();
      exit(1);
    }
  }
	
  putpixel(pScreen, x, y, SDL_MapRGB(pScreen->format, m_palette[0][color], 
				     m_palette[1][color], 
				     m_palette[2][color]));
  if (x == width - 1 && y == height - 1)
    {
      SDL_UnlockSurface(pScreen);
      SDL_UpdateRect(pScreen, 0, 0, width, height);
    }
}
	

// Set the pixel at (x, y) to the given value
// NOTE: The surface must be locked before calling this!
void SDLMandel::putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  // Here p is the address to the pixel we want to set
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
   
  switch(bpp) {
  case 1:
    *p = pixel;
    break;   
  case 2:
    *(Uint16 *)p = pixel;
    break;	
  case 3:
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    } 
    else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }
    break;
  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}
