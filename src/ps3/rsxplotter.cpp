#include <iostream>
#include "ps3/rsxplotter.h"

u32 GetXRGB32(int, int, int);
void putpixel(rsxBuffer *, int, int, u32);

RSXPlotter::RSXPlotter(rsxBuffer *surface,
		Palette &palette) : IPlotter(), _palette(palette)
{
  m_surface = surface;
}

void RSXPlotter::plot(int x, int y, int color) {
	u32 argb = GetXRGB32(_palette.GetR(color), _palette.GetG(color), _palette.GetB(color));
	putpixel(m_surface, x, y, argb);
}

u32 GetXRGB32(int r, int g, int b)
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
