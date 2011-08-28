#include <iostream>
#include "linux/sdlplotter.h"

using std::cerr;

SDLPlotter::SDLPlotter(SDL_Surface *surface,
		       int w, int h,
		       Palette &palette) : IPlotter(), _palette(palette) {
  m_surface = surface;
  width = w;
  height = h;
}

void SDLPlotter::LockSurface() {
	if (SDL_LockSurface(m_surface) != 0) {
		cerr << "Can't get access to screen!!\n";
		SDL_Quit();
		exit(1);
	}
}

void SDLPlotter::UnlockAndUpdateSurface() {
	SDL_UnlockSurface(m_surface);
	SDL_UpdateRect(m_surface, 0, 0, width, height);
}

void SDLPlotter::plot(int x, int y, int color)
{
	putpixel(m_surface, x, y, SDL_MapRGB(m_surface->format,
		  _palette.GetR(color),
		  _palette.GetG(color),
		  _palette.GetB(color)));
}


// Set the pixel at (x, y) to the given value
// NOTE: The surface must be locked before calling this!
void SDLPlotter::putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
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
