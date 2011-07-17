// sdlmandel.cpp
#include <iostream>
#include "sdlmandel.h"
#include <SDL/SDL.h>
using std::cerr;

SDLMandel::SDLMandel(unsigned int iterations, int w, int h, IPlotter &plotter)
  : Mandel(iterations, w, h), m_plotter(plotter)
{
  zoom_start();
}

void SDLMandel::plot(int x, int y, int color)
{
  m_plotter.plot(x, y, color);
}
