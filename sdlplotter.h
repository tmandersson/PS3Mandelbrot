#include "mandel.h"
#include <SDL/SDL.h>

#ifndef SDLPLOTTER_H_
#define SDLPLOTTER_H_

class SDLPlotter : public IPlotter {
 public:
  SDLPlotter(SDL_Surface *surface, int w, int h, int palette[3][256]);
  void plot(int x, int y, int color);
 private:
  SDL_Surface *m_surface;
  int (*m_palette)[256];
  int width, height;
  void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
};

#endif /* SDLPLOTTER_H_ */
