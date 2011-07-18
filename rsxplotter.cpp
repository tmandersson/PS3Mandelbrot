#include <iostream>
#include "rsxplotter.h"

u32 GetARGB32(u16, u16, u16);
void putpixel(rsxBuffer *, int, int, u32);

RSXPlotter::RSXPlotter(rsxBuffer *surface,
		Palette &palette) : IPlotter(), _palette(palette)
{
  m_surface = surface;
}

void RSXPlotter::plot(int x, int y, int color) {
	u32 argb = GetARGB32(_palette.GetR(color), _palette.GetG(color), _palette.GetB(color));
	putpixel(m_surface, x, y, argb);
}

u32 GetARGB32(int r, int g, int b)
{
    u32 result = 0x00000000;
    result += (u32)((r << 16));
    result += (u32)((g << 8));
    result += b;
    return result;
}

void putpixel(rsxBuffer *buffer, int x, int y, u32 color) {
	buffer->ptr[y * buffer->width + x] = color;
}
