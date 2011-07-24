#include "core/mandel.h"
#include "core/palette.h"
#include <SDL/SDL.h>

#ifndef SDLPLOTTER_H_
#define SDLPLOTTER_H_

class SDLPlotter : public IPlotter {
 public:
  SDLPlotter(SDL_Surface *surface, int w, int h, Palette &palette);
  void plot(int x, int y, int color);
 private:
  SDL_Surface *m_surface;
  Palette &_palette;
  int width, height;
  void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
};

#endif /* SDLPLOTTER_H_ */
