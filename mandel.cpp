// mandel.cpp
#include "mandel.h"
const double MinReal = -2.0;
const double MaxReal = 0.6;
const double MinImag = -1.3;
const double MaxImag = 1.3;

Mandel::Mandel( unsigned int iterations,
		int w, int h, IPlotter &plotter) : m_plotter(plotter)
{
  width = w;
  height = h;
  oldminre = minre = MinReal;
  oldmaxre = maxre = MaxReal;
  oldminim = minim = MinImag;
  oldmaxim = maxim = MaxImag;
  maxiter = iterations;

  havepainted = false; // haven't drawn the fractal yet
}

void Mandel::calcmandel()
{
  int x, y;
  complex pos(minre, maxim);
  havepainted = true;
  double xstep = (maxre - minre) / width;
  double ystep = xstep;
  // or ystep = (maxim - minim) / height if we allow the scales to be
  // unproportional

  // color constant (how many steps the color changes per iteration)
  double colorconst = 256 / (double) maxiter; // shouldn't be done here
  unsigned int c_iterations;
  for (y = 0; y < height; y++)
    {
      if (y > 0)
	pos -= complex(0, ystep);
		  
      pos = complex(minre, pos.imag()); // start with the first pixel on the row
      for (x = 0; x < width; x++)
	{
	  if (x > 0)
	    pos += xstep;
			
	  // plot the pixel if it doesn't belong to the Mandel set
	  if ( (c_iterations = mandeliteration(pos, maxiter)) )	       
	    m_plotter.plot(x, y, (c_iterations * colorconst) < 1
		 ? 1: (int) (c_iterations * colorconst));
	  else
	    m_plotter.plot(x, y, 0);
	}
    }
}

void Mandel::zoom(double minr, double maxr, double mini, double maxi)
{
  if (maxr > minr && maxi > mini) {
    oldminre = minre; // save the old values, to be able too zoom back
    oldmaxre = maxre;
    oldminim = minim;
    oldmaxim = maxim;
    minre = minr;
    maxre = maxr;
    minim = mini;
    maxim = maxi;
      
    // paint the fractal
    calcmandel();
  }
}

void Mandel::zoom_cord(int fromx, int fromy, int tox, int toy)
{
  double newminre, newmaxre, newminim, newmaxim;
   
  // calculate new view range
  if (tox > fromx && toy > fromy) {
    double xstep = (maxre - minre) / width;
    double ystep = (maxim - minim) / height;
     
    newminre = minre + fromx * xstep;
    newmaxre = maxre - (width - tox) * xstep;
    newminim = minim + (height - (fromy + tox - fromx)) * ystep;
    newmaxim = maxim - fromy * ystep;

    // zoom with the new values
    zoom(newminre, newmaxre, newminim, newmaxim);
  }
}

void Mandel::zoom_back()
{
  if (oldminre != minre && oldmaxre != maxre) 
    {
      minre = oldminre;
      maxre = oldmaxre;
      minim = oldminim;
      maxim = oldmaxim;
      zoom(minre, maxre, minim, maxim);
    }
}

// Iterate Zn+1 = Zn� + C till we now if the iterations is gonna reach infinity
// or not.
// If we don't reach infinity then the function returns the number of 
// iterations that were needed and the complex constant C belongs to the 
// Mandel set.
// We start with n = 0 (Z0 = 0)
// C is the first argument (it also stands for wich pixel on the screen that 
// we're calculating)
// the second argument is the maximal number of iterations before we consider
// C a part of the Mandel set.
unsigned int Mandel::mandeliteration(complex &c, unsigned int maxiterations)
{
  complex z(0,0);
  unsigned iterations;
  bool infinity = false;
	
  // Stop when we maximum number of iterations is reached (part of Mandel set)
  // or when we're certain that the iteration is going to reach infinity.
  for (iterations = 0; iterations < maxiterations && !infinity; iterations++)
    {
      z = z*z + c;

      // We now that everything outside a circle with the radius of
      // 2 is outside the Mandel set.		
      // Thus if the abs(z) > 2 then the iterations is going to reach infinity
      if ((z.real()*z.real() + z.imag()*z.imag()) > 4)
	infinity = true;
    }		
	
  // if the iteration don't reach infinity then C is part of the	Mandel set
  if (!infinity)
    return 0;
  else
    return iterations;
}
