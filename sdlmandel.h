#include "SDL/SDL.h"
#include "mandel.h"

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

class SDLPlotter : public IPlotter {
 public:
  SDLPlotter(SDL_Surface *surface, int w, int h, int palette[3][256]);
  void plot(int x, int y, int color);
 private:
  SDL_Surface *m_surface;
  int (*m_palette)[256];
  int width, height;
  void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
};
