#include "SDL/SDL.h"
#include "mandel.h"

class SDLMandel : public Mandel
{
	public:
		SDLMandel(unsigned int iterations = 100, int w = 640, int h = 480);	
		~SDLMandel();
	protected:
		// override base-class
		void plot(int x, int y, int color);
		//void repaint();
	private:
		SDL_Surface *pScreen;
		// temporary zoom variables
		int fromx;
		int fromy;
		
		void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
};
