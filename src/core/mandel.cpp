#include "core/mandel.h"
#include <time.h>
#include "core/mftb_profiling.h"

#include <core/fractal_params.h>
#include <sys/spu.h>
#include "spu_bin.h"
#define ptr2ea(x)			((u64)((void*)(x)))

const unsigned int ITERATIONS = 256;

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

void Mandel::paint()
{
	mftbStart(start);
	time_t time_start = time(NULL);

	_plotter.LockSurface();

	_have_painted = true;
	_x_step = (_max_re - _min_re) / _width;
	_y_step = (_max_im - _min_im) / _height;

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
