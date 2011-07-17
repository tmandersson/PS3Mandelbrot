#ifndef PALETTE_H_
#define PALETTE_H_

class Palette {
public:
	Palette();
	int GetR(int color);
	int GetG(int color);
	int GetB(int color);
private:
	int palette[3][256];
	void CreatePalette();
};

#endif /* PALETTE_H_ */
