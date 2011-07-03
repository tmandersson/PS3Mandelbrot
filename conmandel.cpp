#include <iostream>
#include "mandel.h"

using std::cout;
using std::endl;

class ConMandel : public Mandel
{
public:
  ConMandel( unsigned int its, int w, int h) : Mandel(its, w, h) {}
  void print() { calcmandel(); }
protected:
  virtual void plot(int x, int y, int color);
};

void ConMandel::plot(int x, int y, int color)
{
  static int old_y = -1;
  if (old_y == -1 || y != old_y) cout << endl;
  old_y = y;
  if (color == 0) cout << " "; else cout <<"x";
}

int main(int argc, char **argv)
{
  ConMandel TestMandel(1000, 80, 80);  
  TestMandel.print();
  cout << endl;
  return 0;
}
