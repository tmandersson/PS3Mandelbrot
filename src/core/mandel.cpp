// mandel.cpp
#include "core/mandel.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "core/mftb_profiling.h"

const unsigned int ITERATIONS = 256;

Mandel::Mandel(int width, int height, IPlotter &plotter) : _plotter(plotter)
{
	_width = width;
	_height = height;
	_max_iterations = ITERATIONS;
	_old_min_re = _old_max_re = _old_min_im = _old_max_im = 0;
	_have_painted = false; // haven't drawn the fractal yet

	_results = (unsigned int*) calloc(_width*_height, sizeof(unsigned int));
	if (!_results)
	{
		printf("Memory allocation failed!\n");
		exit(1);
	}
}

Mandel::~Mandel()
{
	free(_results);
}

void Mandel::paint()
{
	mftbStart(start);
	time_t time_start = time(NULL);

	int x, y;
	double re;
	double im = _max_im;
	_have_painted = true;
	double x_step = (_max_re - _min_re) / _width;
	double y_step = (_max_im - _min_im) / _height;

	for (y = 0; y < _height; y++) {
		if (y > 0) {
			im -= y_step;
		}
		re = _min_re; // start with the first pixel on the row
		calculate_row(re, im, x_step, &_results[y*_width]);
	}

	for (y = 0; y < _height; y++) {
		for (x = 0; x < _width; x++) {
			// plot the pixel with colour if it doesn't belong to the Mandel set
			unsigned int iterations = _results[y*_width + x];
			if ( iterations != 0)
				_plotter.plot(x, y, (iterations % 255) + 1);
			else
				_plotter.plot(x, y, 0);
		}
	}

	time_t time_end = time(NULL);
	long unsigned time = difftime(time_end, time_start);
	long unsigned pixels = (_width * _height);
	printf("Start time is: %lu End time is: %lu\n", time_start, time_end);
	printf("Time elapsed: %lus\n", time);
	if (time > 0)
		printf("Pixels calculated per second: %.2Lf\n", (long double) (pixels / time));

    mftbStop(start,stop);
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

void Mandel::zoom_coord(int from_x, int from_y, int to_x, int to_y)
{
	double new_min_re, new_max_re, new_min_im, new_max_im;

	// calculate new view range
	if (to_x > from_x && to_y > from_y) {
		double x_step = (_max_re - _min_re) / _width;
		double y_step = (_max_im - _min_im) / _height;

		new_min_re = _min_re + from_x * x_step;
		new_max_re = _max_re - (_width - to_x) * x_step;
		new_min_im = _min_im + from_y * y_step;
		new_max_im = _max_im - (_height - to_y) * y_step;
		// new_min_im = _min_im + (_height - (from_y + to_x - from_x)) * y_step;
		// new_max_im = _max_im - from_y * y_step;

		// zoom with the new values
		zoom(new_min_re, new_max_re, new_min_im, new_max_im);
		printf("New values: %f %f %f %f (minr, maxr, mini, maxi)", new_min_re, new_max_re, new_min_im, new_max_im);
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

void Mandel::calculate_row(double re, double im, double x_step, unsigned int *results) {
	int x;
	for (x = 0; x < _width; x++) {
		if (x > 0)
			re += x_step;

		// save number of iterations
		results[x] = calculate(re, im);
	}
}

// Iterate Zn+1 = Zn^2 + C till we now if the iterations is gonna reach infinity
// or not.
// If we don't reach infinity then the function returns the number of 
// iterations that were needed and the complex constant C belongs to the 
// Mandel set.
// We start with n = 0 (Z0 = 0)
// C is the first argument (it also stands for wich pixel on the screen that 
// we're calculating)
// the second argument is the maximal number of iterations before we consider
// C a part of the Mandel set.
unsigned int Mandel::calculate(double c_re, double c_im)
{
	double z_re, z_im;
	z_re = z_im = 0;
	unsigned iterations;
	bool infinity = false;
	
	// Stop when we maximum number of iterations is reached (part of Mandel set)
	// or when we're certain that the iteration is going to reach infinity.
	for (iterations = 0; iterations < _max_iterations && !infinity; iterations++) {
		// z = z*z + c;
		double new_z_re, new_z_im;
		new_z_re = ((z_re*z_re) - (z_im*z_im) + c_re);
		new_z_im = ((z_re*z_im * 2) + c_im);
		z_re = new_z_re;
		z_im = new_z_im;

		// We now that everything outside a circle with the radius of
		// 2 is outside the Mandel set.
		// Thus if the abs(z) > 2 then the iterations is going to reach infinity
		if ((z_re*z_re + z_im*z_im) > 4)
			infinity = true;
	}
	
	// if the iteration don't reach infinity then C is part of the	Mandel set
	if (!infinity)
		return 0;
	else
		return iterations;
}
