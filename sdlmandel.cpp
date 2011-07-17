// sdlmandel.cpp
#include <iostream>
#include "sdlmandel.h"
#include <SDL/SDL.h>
using std::cerr;

SDLMandel::SDLMandel(unsigned int iterations, int w, int h, IPlotter &plotter)
  : Mandel(iterations, w, h, plotter), m_plotter(plotter) {}
