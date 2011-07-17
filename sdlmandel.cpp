// sdlmandel.cpp
#include <iostream>
#include "sdlmandel.h"
#include <SDL/SDL.h>
using std::cerr;

SDLMandel::SDLMandel(unsigned int iterations, int w, int h, IPlotter &plotter)
  : Mandel(iterations, w, h), m_plotter(plotter)
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
  m_plotter.plot(x, y, color);
}
