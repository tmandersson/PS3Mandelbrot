#include "core/mandel.h"
#include "core/palette.h"
#include "ps3/rsxutil.h"

#ifndef RSXPLOTTER_H_
#define RSXPLOTTER_H_

class RSXPlotter : public IPlotter {
 public:
	RSXPlotter(rsxBuffer *surface, Palette &palette);
	void plot(int x, int y, int color);
 private:
	rsxBuffer *m_surface;
	Palette &_palette;
};

#endif /* RSXPLOTTER_H_ */
