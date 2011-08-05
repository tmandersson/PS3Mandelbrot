#include "core/mandel.h"
#include "linux/sdlplotter.h"
#include <SDL/SDL.h>
#include <iostream>
using namespace std;

const int WIDTH = 1920;
const int HEIGHT = 1080;
const unsigned int ITERATIONS = 50;

SDL_Surface *InitSDL();

int main(int argc, char *argv[])
{
	SDL_Surface *surface = InitSDL();

	Palette palette = Palette();
	SDLPlotter plotter = SDLPlotter(surface, WIDTH, HEIGHT, palette);
	Mandel mand = Mandel(ITERATIONS, WIDTH, HEIGHT, plotter);


	mand.zoom(-3.0, 1.6, -1.3, 1.3);
	getchar();

//	while(1) {
//		mand.zoom_coord(192, 108, 1920-192, 1080-108);
//	}
	SDL_Quit();
}

SDL_Surface *InitSDL()
{
	// Initialise defaults, Video
	if ((SDL_Init(SDL_Init(SDL_INIT_VIDEO))==-1)) {
		cerr << "Could not initialise SDL: " << SDL_GetError() << ".\n";
		exit(1);
	}

	// Initialise the display
	// requesting a hardware surface
	SDL_Surface *surface = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE);
	if ( surface == NULL ) {
		cerr << "Couldn't set " << WIDTH << "x" << HEIGHT << "x32 video mode: "
		 << SDL_GetError() << ".\n";
		exit(1);
	}

	return surface;
}
