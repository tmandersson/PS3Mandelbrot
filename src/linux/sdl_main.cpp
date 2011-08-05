#include "core/mandel.h"
#include "linux/sdlplotter.h"
#include <SDL/SDL.h>
#include <iostream>
using namespace std;

const int WIDTH = 1280;
const int HEIGHT = 720;
const unsigned int ITERATIONS = 50;

int kbhit();
SDL_Surface *InitSDL();

int main(int argc, char *argv[])
{
	SDL_Surface *surface = InitSDL();

	Palette palette = Palette();
	SDLPlotter plotter = SDLPlotter(surface, WIDTH, HEIGHT, palette);
	Mandel mand = Mandel(ITERATIONS, WIDTH, HEIGHT, plotter);


	mand.zoom(-3.0, 1.6, -1.3, 1.3);
	mand.zoom_coord(WIDTH*0.1-23, HEIGHT*0.1+90, WIDTH-WIDTH*0.1-23, HEIGHT-HEIGHT*0.1+90);

	while(!kbhit()) {
		mand.zoom_coord(WIDTH*0.01, HEIGHT*0.01, WIDTH-WIDTH*0.01, HEIGHT-HEIGHT*0.01);
	}
	SDL_Quit();
}

int kbhit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
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
