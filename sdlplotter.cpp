#include <iostream>
#include "sdlplotter.h"

using std::cerr;

SDLPlotter::SDLPlotter(SDL_Surface *surface,
		       int w, int h,
		       int palette[3][256]) : IPlotter() {
  m_surface = surface;
  m_palette = palette;
  width = w;
  height = h;
}

void SDLPlotter::plot(int x, int y, int color)
{
  if (x == 0 && y == 0) {
    // lock the screen
    if (SDL_LockSurface(m_surface) != 0) {
      cerr << "Can't get access to screen!!\n";
      SDL_Quit();
      exit(1);
    }
  }

  putpixel(m_surface, x, y, SDL_MapRGB(m_surface->format, m_palette[0][color],
				     m_palette[1][color],
				     m_palette[2][color]));
  if (x == width - 1 && y == height - 1)
    {
      SDL_UnlockSurface(m_surface);
      SDL_UpdateRect(m_surface, 0, 0, width, height);
    }
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
