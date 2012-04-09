#include "core/mandel.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "core/mftb_profiling.h"
#include <unistd.h>
#include <sys/thread.h>

#include <core/fractal_params.h>
#include <sys/spu.h>
#include "spu_bin.h"
#define ptr2ea(x)			((u64)((void*)(x)))

const int CALCULATION_THREADS = 5;

typedef sys_ppu_thread_t pthread_t;

int pthread_create (pthread_t *id, void *attr, void *(* __start_routine) (void *), void * __arg)
{
	void (* routine) (void *) = (void (*)(void *)) __start_routine;
	return sysThreadCreate(id, routine, __arg, 1001, 0x100000, THREAD_JOINABLE, NULL);
}

void pthread_exit (void *__retval) {
	sysThreadExit(0);
}

int pthread_join (pthread_t __th, void **__thread_return) {
	u64 ret_val;
	return sysThreadJoin(__th, &ret_val);
}

const unsigned int ITERATIONS = 256;

struct section_params {
	int start_pixel_row;
	int end_pixel_row;
	double start_im;
	Mandel *mandel_object;
};

struct thread_data {
	u32 id;
	sysSpuThreadArgument arg;
	struct fractal_params params __attribute__((aligned(128)));
};

const int thread_count = 6;

void calculate_with_spus(void *result, struct fractal_params *params) {
	static struct thread_data thread[thread_count];
	sysSpuImage image;
	u32 group_id;
	u32 cause, status;
	int priority = 100;
	sysSpuThreadGroupAttribute grpattr = { 7+1, ptr2ea("fractal"), 0, 0 };
	sysSpuThreadAttribute attr = { ptr2ea("f_thread"), 8+1, SPU_THREAD_ATTR_NONE };

	sysSpuInitialize(thread_count, 0);
	sysSpuThreadGroupCreate(&group_id, thread_count, priority, &grpattr);
	sysSpuImageImport(&image, spu_bin, SPU_IMAGE_PROTECT);

	for(int index = 0; index < thread_count; index++) {
		thread[index].params.pixel_width = params->pixel_width;
		thread[index].params.pixel_height = params->pixel_height/thread_count;
		thread[index].params.min_re = params->min_re;
		thread[index].params.max_im = params->max_im - (params->y_step * thread[index].params.pixel_height * index);
		thread[index].params.x_step = params->x_step;
		thread[index].params.y_step = params->y_step;
		thread[index].params.max_iterations = params->max_iterations;

		void *thread_result = result + ((params->pixel_height/thread_count) * params->pixel_width) * sizeof(int) * index;
		thread[index].arg.arg0 = ptr2ea(thread_result);
		thread[index].arg.arg1 = ptr2ea(&thread[index].params);
		thread[index].arg.arg2 = 0;
		thread[index].arg.arg3 = 0;
		sysSpuThreadInitialize(&thread[index].id, group_id, index, &image, &attr, &thread[index].arg);
	}

	//mftbStart(start);
	sysSpuThreadGroupStart(group_id);
	sysSpuThreadGroupJoin(group_id, &cause, &status);
	//mftbStop(start,stop);

	sysSpuThreadGroupDestroy(group_id);
	sysSpuImageClose(&image);
}

Mandel::Mandel(int width, int height, IPlotter &plotter) : _plotter(plotter)
{
	_width = width;
	_height = height;
	_max_iterations = ITERATIONS;
	_old_min_re = _old_max_re = _old_min_im = _old_max_im = 0;
	_have_painted = false; // haven't drawn the fractal yet
}

void *call_calculate_section(void *params)  {
	Mandel *obj = ((section_params *)params)->mandel_object;
	obj->calculate_section(params);
	pthread_exit(NULL);
	return NULL;
}

void Mandel::paint()
{
	mftbStart(start);
	time_t time_start = time(NULL);

	_plotter.LockSurface();

	_have_painted = true;
	_x_step = (_max_re - _min_re) / _width;
	_y_step = (_max_im - _min_im) / _height;

	int sections = CALCULATION_THREADS;

	int section_limit;
	int old_section_limit;
	section_limit = 0;

	pthread_t threads[sections];
	section_params params[sections];
	for (int section_id = 0; section_id < sections; section_id++)
	{
		old_section_limit = section_limit;
		section_limit = (_height/sections)*(section_id + 1);

		params[section_id].mandel_object = this;
		params[section_id].start_pixel_row = old_section_limit;
		params[section_id].end_pixel_row = section_limit;
		params[section_id].start_im = _max_im - (old_section_limit * _y_step);

	    int rc = pthread_create(&threads[section_id], NULL, call_calculate_section, (void *)&params[section_id]);
	    if (rc) {
	    	printf("ERROR; return code from pthread_create() is %d\n", rc);
	    	exit(-1);
	    }
	}

	for (int section_id = 0; section_id < sections; section_id++)
	{
		if (pthread_join(threads[section_id], NULL) != 0) {
			printf("ERROR: Thread join failed!\n");
		}
	}

	static struct fractal_params fract_params __attribute__((aligned(128)));
	fract_params.pixel_width = _width;
	fract_params.pixel_height = _height;
	fract_params.min_re = _min_re;
	fract_params.max_im = _max_im;
	fract_params.x_step = _x_step;
	fract_params.y_step = _y_step;
	fract_params.max_iterations = _max_iterations;
	calculate_with_spus(_plotter.getSurface()->ptr, &fract_params);

	_plotter.UnlockAndUpdateSurface();

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
		double _x_step = (_max_re - _min_re) / _width;
		double _y_step = (_max_im - _min_im) / _height;

		new_min_re = _min_re + from_x * _x_step;
		new_max_re = _max_re - (_width - to_x) * _x_step;
		new_min_im = _min_im + from_y * _y_step;
		new_max_im = _max_im - (_height - to_y) * _y_step;
		// new_min_im = _min_im + (_height - (from_y + to_x - from_x)) * _y_step;
		// new_max_im = _max_im - from_y * _y_step;

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

void Mandel::calculate_section(void *params) {
	section_params *p;
	p = (section_params *) params;
	int start_pixel_row = p->start_pixel_row;
	int end_pixel_row = p->end_pixel_row;
	double start_im = p->start_im;

	int y;
	double re;
	double im = start_im;
	for (y = start_pixel_row; y < end_pixel_row; y++) {
		if (y > start_pixel_row) {
			im -= _y_step;
		}
		re = _min_re; // start with the first pixel on the row
		calculate_row(re, im, y);
	}
}

void Mandel::calculate_row(double re, double im, int y) {
	int x;
	for (x = 0; x < _width; x++) {
		if (x > 0)
			re += _x_step;

		unsigned int iterations = calculate(re, im);

		if ( iterations != 0)
			_plotter.plot(x, y, (iterations % 255) + 1);
		else
			_plotter.plot(x, y, 0);
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
