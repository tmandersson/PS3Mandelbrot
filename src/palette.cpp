#include "palette.h"

Palette::Palette() {
	CreatePalette();
}

int Palette::GetR(int color) {
	return palette[0][color];
}

int Palette::GetG(int color) {
	return palette[1][color];
}

int Palette::GetB(int color) {
	return palette[2][color];
}

void Palette::CreatePalette()
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
      palette[0][i] = temp[i].r;
      palette[1][i] = temp[i].g;
      palette[2][i] = temp[i].b;
    }
}
