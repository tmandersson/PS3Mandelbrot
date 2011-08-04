// mandel.cpp
#include "core/mandel.h"
#include <stdio.h>
#include <time.h>
#include "core/mftb_profiling.h"

Mandel::Mandel(unsigned int iterations,
		int width, int height, IPlotter &plotter) : _plotter(plotter)
{
	_width = width;
	_height = height;
	_max_iterations = iterations;
	_old_min_re = _old_max_re = _old_min_im = _old_max_im = 0;
	_have_painted = false; // haven't drawn the fractal yet
}

void Mandel::paint()
{
	time_t start = time(NULL);

	int x, y;
	complex pos(_min_re, _max_im);
	_have_painted = true;
	double x_step = (_max_re - _min_re) / _width;
	double y_step = x_step;

	// color constant == (how many steps the color changes per iteration)
	double color_constant = 256 / (double) _max_iterations; // shouldn't be done here
	unsigned int iterations;
	for (y = 0; y < _height; y++) {
		if (y > 0)
			pos -= complex(0, y_step);
		  
		pos = complex(_min_re, pos.imag()); // start with the first pixel on the row
		for (x = 0; x < _width; x++) {
			if (x > 0)
				pos += x_step;

			// plot the pixel if it doesn't belong to the Mandel set
			if ( (iterations = calculate(pos)) )
				_plotter.plot(x, y, (iterations * color_constant) < 1 ? 1: (int) (iterations * color_constant));
			else
				_plotter.plot(x, y, 0);
		}
	}

	time_t end = time(NULL);
	long unsigned time = difftime(end, start);;
	long unsigned pixels = (_width * _height);
	printf("Start time is: %lu End time is: %lu\n", start, end);
	printf("Time elapsed: %lus\n", time);
    printf("Pixels calculated per second: %.2Lf\n", (long double) (pixels / time));
}

void Mandel::zoom(double min_re, double max_re, double min_im, double max_im)
{
	if (max_re > min_re && max_im > min_im) {
		_old_min_re = _min_re; // save the old values, to be able too zoom back
		_old_max_re = _max_re;
		_old_min_im = _min_im;
		_old_max_im = _max_im;
		_min_re = min_re;
		_max_re = max_re;
		_min_im = min_im;
		_max_im = max_im;

		// paint the fractal
		paint();
	}
}

void Mandel::zoom_cord(int from_x, int from_y, int to_x, int to_y)
{
	double new_min_re, new_max_re, new_min_im, new_max_im;

	// calculate new view range
	if (to_x > from_x && to_y > from_y) {
		double x_step = (_max_re - _min_re) / _width;
		double y_step = (_max_im - _min_im) / _height;

		new_min_re = _min_re + from_x * x_step;
		new_max_re = _max_re - (_width - to_x) * x_step;
		new_min_im = _min_im + (_height - (from_y + to_x - from_x)) * y_step;
		new_max_im = _max_im - from_y * y_step;

		// zoom with the new values
		zoom(new_min_re, new_max_re, new_min_im, new_max_im);
	}
}

void Mandel::zoom_back()
{
	if (_old_min_re == 0 && _old_max_re == 0 && _old_min_im == 0 && _old_max_im == 0)
		return;

	if (_old_min_re != _min_re && _old_max_re != _max_re) {
		_min_re = _old_min_re;
		_max_re = _old_max_re;
		_min_im = _old_min_im;
		_max_im = _old_max_im;
		zoom(_min_re, _max_re, _min_im, _max_im);
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
unsigned int Mandel::calculate(complex &c)
{
	complex z(0,0);
	unsigned iterations;
	bool infinity = false;
	
	// Stop when we maximum number of iterations is reached (part of Mandel set)
	// or when we're certain that the iteration is going to reach infinity.
	for (iterations = 0; iterations < _max_iterations && !infinity; iterations++) {
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
