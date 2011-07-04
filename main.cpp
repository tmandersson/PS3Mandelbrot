// Main.cpp
#include "sdlmandel.h"
#include <iostream>
using namespace std;

const int WIDTH = 1280;
const int HEIGHT = 720;
const unsigned int ITERATIONS = 500;

int m_palette[3][256];

void create_palette()
{
  struct color 
  {
    int r;
    int g;
    int b;
  };
	
  color temp[256];
	
  int i;
  temp[0].r = temp[0].g = temp[0].b = 0; // Black (represents the Mandel set)
  for (i = 1; i < 50; i++) 
    {
      temp[i].r = 0;
      temp[i].g = 175 - i*3; // 172 - 28
      temp[i].b = 252;
    }
  for (i = 50; i < 144; i++) 
    {
      temp[i].r = 4 + (i - 50) * 2; // 4 - 190
      temp[i].g = 30 + (i - 50); // 30 - 173
      temp[i].b = 248 - (i - 50) * 2; // 248 - 62
    }
  for (i = 144; i < 208; i++) 
    {
      temp[i].r = 192 + (i - 144); // 192 - 255
      temp[i].g = (175 + (i - 144) * 2) < 256 
	? (175 + (i - 144) * 2) : 255; // 175 - 255
      temp[i].b = (60 - (i - 144)) > 0 
	? 60 - (i - 144) : 0; // 60 - 0
    }
   
  for (i = 208; i < 256; i++) 
    {
      temp[i].r = temp[i].g = 255;
      temp[i].b = 114 + (i - 208) * 3; // 114 - 255
    }
   
  // fill the palette array
  for (i = 0; i < 256; i++) 
    {
      m_palette[0][i] = temp[i].r;
      m_palette[1][i] = temp[i].g;
      m_palette[2][i] = temp[i].b;
    }
}

int main(int argc, char *argv[])
{
  create_palette();
  
  // Initialize defaults, Video
  if ((SDL_Init(SDL_Init(SDL_INIT_VIDEO))==-1)) {
    cerr << "Could not initialize SDL: " << SDL_GetError() << ".\n";
    exit(1);
  }
	
  // Initialize the display
  // requesting a hardware surface
  SDL_Surface *surface = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE);
  if ( surface == NULL ) {
    cerr << "Couldn't set " << WIDTH << "x" << HEIGHT << "x32 video mode: " 
	 << SDL_GetError() << ".\n";
    exit(1);
  }

  SDLPlotter plotter = SDLPlotter(surface, WIDTH, HEIGHT, m_palette);
  SDLMandel mand = SDLMandel(ITERATIONS, WIDTH, HEIGHT, plotter);

  mand.zoom(-3.0, 1.6, -1.3, 1.3);
  char test;
  cin >> test;
}
