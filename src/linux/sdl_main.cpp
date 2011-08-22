#include "core/mandel.h"
#include "linux/sdlplotter.h"
#include <SDL/SDL.h>
#include <iostream>
using namespace std;

const int WIDTH = 1920;
const int HEIGHT = 1080;

int kbhit();
SDL_Surface *InitSDL();

int main(int argc, char *argv[])
{
	SDL_Surface *surface = InitSDL();

	Palette palette;
	SDLPlotter plotter(surface, WIDTH, HEIGHT, palette);
	Mandel mand(WIDTH, HEIGHT, plotter);

	// -0.743643887037158704752191506114774, 0.131825904205311970493132056385139
	double start_real = -0.743643887037158704752191506114774;
	double start_imag = 0.131825904205311970493132056385139;
	double x_aspect = 16;
	double y_aspect = 9;
	double offset_real = x_aspect * 0.1;
	double offset_imag = y_aspect * 0.1;
	mand.zoom(start_real-offset_real, start_real+offset_real, start_imag-offset_imag, start_imag+offset_imag);

	while(!kbhit()) {
		mand.zoom_coord(x_aspect, y_aspect, WIDTH-x_aspect, HEIGHT-y_aspect);
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
