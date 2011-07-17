#include <SDL/SDL.h>
#include "mandel.h"

#ifndef SDLMANDEL_H_
#define SDLMANDEL_H_

class IPlotter {
 public:
  virtual void plot(int x, int y, int color) = 0;
};

class SDLMandel : public Mandel {
 public:
  SDLMandel(unsigned int iterations, int w, int h, IPlotter &plotter);
  ~SDLMandel();
 protected:
  // override base-class
  void plot(int x, int y, int color);
  //void repaint();
 private:
  IPlotter &m_plotter;
  SDL_Surface *pScreen;
  // temporary zoom variables
  int fromx;
  int fromy;
		
  void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
};

#endif /* SDLMANDEL_H_ */
