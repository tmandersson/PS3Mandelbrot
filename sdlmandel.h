#include <SDL/SDL.h>
#include "mandel.h"

#ifndef SDLMANDEL_H_
#define SDLMANDEL_H_

class SDLMandel : public Mandel {
 public:
  SDLMandel(unsigned int iterations, int w, int h, IPlotter &plotter);
 protected:
  // override base-class
  void plot(int x, int y, int color);
 private:
  IPlotter &m_plotter;
};

#endif /* SDLMANDEL_H_ */
