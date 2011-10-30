#include <stdio.h>
#include <sys/spu.h>
#include "spu_bin.h"
#include <core/fractal_params.h>

#include <ppu-lv2.h>
#include <malloc.h>
#include <io/pad.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include "ps3/rsxutil.h"
#include "ps3/rsxplotter.h"
#include <sys/process.h>
SYS_PROCESS_PARAM(1001,0x400000);

const int WIDTH = 20;
const int HEIGHT = 20;
const unsigned int MAX_ITERATIONS = 256;

#define ptr2ea(x)			((u64)((void*)(x)))

void *allocate_result_buffer(struct fractal_params *params);
void calculate_with_spus(void *result, struct fractal_params *params);
void calculate_fractal(int *result, struct fractal_params *params);
void print_values_wh(int *result, int width, int height);
void print_values(int *result);
void show_fractal_on_screen(int *result, int width, int height);

int main(int argc, char* argv[]) {
	printf("\nNON SPU CODE:\n");

	double start_real = -0.743643887037158704752191506114774;
	double start_imag = 0.131825904205311970493132056385139;
	double x_aspect = 1;
	double y_aspect = 1;
	double offset_real = x_aspect * 1.2;
	double offset_imag = y_aspect * 1.2;

	double min_re, max_re, min_im, max_im;
	min_re = start_real-offset_real;
	max_re = start_real+offset_real;
	min_im = start_imag-offset_imag;
	max_im = start_imag+offset_imag;
	double x_step = (max_re - min_re) / WIDTH;
	double y_step = (max_im - min_im) / HEIGHT;
	int result[HEIGHT*WIDTH];

	static struct fractal_params params __attribute__((aligned(128)));
	params.pixel_width = WIDTH;
	params.pixel_height = HEIGHT;
	params.min_re = min_re;
	params.max_im = max_im;
	params.x_step = x_step;
	params.y_step = y_step;
	params.max_iterations = MAX_ITERATIONS;

	calculate_fractal(result, &params);
	print_values(result);

	printf("\n\nSPU CODE with bigger fractal:\n");
	int width = 1920;
	int height = 1080;
	x_step = (max_re - min_re) / width;
	y_step = (max_im - min_im) / height;
	params.pixel_width = width;
	params.pixel_height = height;
	params.min_re = min_re;
	params.max_im = max_im;
	params.x_step = x_step;
	params.y_step = y_step;
	params.max_iterations = MAX_ITERATIONS;

	void *big_result = allocate_result_buffer(&params);
	calculate_with_spus(big_result, &params);
	printf("Calculated %i number of pixels...", height*width);
	show_fractal_on_screen((int *)big_result, width, height);
	free(big_result);

	printf("\n\nExiting!\n");
	return 0;
}

void *allocate_result_buffer(struct fractal_params *params) {
	int size = sizeof(int) * params->pixel_width * params->pixel_height;
	if (size % 16 > 0) // need to dma transfer full blocks of 16 bytes
		size += 16 - size % 16;
	void *result_buffer = malloc(size);

	int *p = (int *) result_buffer;
	for (int i=0; i<params->pixel_width*params->pixel_height; i++)
		p[i] = 0;

	return result_buffer;
}

struct thread_data {
	u32 id;
};

void calculate_with_spus(void *result, struct fractal_params *params) {
	int thread_count = 2;
	struct thread_data thread[thread_count];
	sysSpuImage image;
	u32 group_id;
	u32 cause, status;
	int priority = 100;
	sysSpuThreadGroupAttribute grpattr = { 7+1, ptr2ea("fractal"), 0, 0 };
	sysSpuThreadAttribute attr1 = { ptr2ea("f_threa1"), 8+1, SPU_THREAD_ATTR_NONE };
	sysSpuThreadAttribute attr2 = { ptr2ea("f_threa2"), 8+1, SPU_THREAD_ATTR_NONE };
	sysSpuThreadArgument arg1 = { 0, 0, 0, 0 };
	sysSpuThreadArgument arg2 = { 0, 0, 0, 0 };

	sysSpuInitialize(6, 0);
	sysSpuThreadGroupCreate(&group_id, thread_count, priority, &grpattr);
	sysSpuImageImport(&image, spu_bin, SPU_IMAGE_PROTECT);

	int index = 0;
	static struct fractal_params params1 __attribute__((aligned(128)));
	params1.pixel_width = params->pixel_width;
	params1.pixel_height = params->pixel_height/6;
	params1.min_re = params->min_re;
	params1.max_im = params->max_im - (params->y_step * params1.pixel_height * index);
	params1.x_step = params->x_step;
	params1.y_step = params->y_step;
	params1.max_iterations = params->max_iterations;

	result += ((params->pixel_height/6) * params->pixel_width) * sizeof(int) * index;
	arg1.arg0 = ptr2ea(result);
	arg1.arg1 = ptr2ea(&params1);
	sysSpuThreadInitialize(&thread[index].id, group_id, index, &image, &attr1, &arg1);

	index = 1;
	static struct fractal_params params2 __attribute__((aligned(128)));
	params2.pixel_width = params->pixel_width;
	params2.pixel_height = params->pixel_height/6;
	params2.min_re = params->min_re;
	params2.max_im = params->max_im - (params->y_step * params2.pixel_height * index);
	params2.x_step = params->x_step;
	params2.y_step = params->y_step;
	params2.max_iterations = params->max_iterations;

	result += ((params->pixel_height/6) * params->pixel_width) * sizeof(int) * index;
	arg2.arg0 = ptr2ea(result);
	arg2.arg1 = ptr2ea(&params2);
	sysSpuThreadInitialize(&thread[index].id, group_id, index, &image, &attr2, &arg2);

	sysSpuThreadGroupStart(group_id);
	sysSpuThreadGroupJoin(group_id, &cause, &status);

	sysSpuThreadGroupDestroy(group_id);
	sysSpuImageClose(&image);
}

void print_values_wh(int *result, int width, int height) {
	for (int y=0; y<height; y++) {
		printf("Line: %i\n", y);
		for (int x=0; x<width; x++)
		{
			int value = result[(y*width)+x];
			if (value == 0)
				printf("    ");
			else
				printf("%.3Xh", result[(y*width)+x]);
		}
		printf("\n");
	}
}

void print_values(int *result) {
	print_values_wh(result, WIDTH, HEIGHT);
}

unsigned int calculate(double c_re, double c_im, unsigned int max_iterations)
{
	double z_re, z_im;
	z_re = z_im = 0;
	unsigned iterations;
	bool infinity = false;

	// Stop when we maximum number of iterations is reached (part of Mandel set)
	// or when we're certain that the iteration is going to reach infinity.
	for (iterations = 0; iterations < max_iterations && !infinity; iterations++) {
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

void calculate_fractal(int *result, struct fractal_params *params) {
	double re;
	double im = params->max_im;
	for (int y = 0; y < params->pixel_height; y++) {
		if (y > 0) {
			im -= params->y_step;
		}
		re = params->min_re; // start with the first pixel on the row

		for (int x = 0; x < params->pixel_width; x++) {
			if (x > 0)
				re += params->x_step;

			unsigned int iterations = calculate(re, im, params->max_iterations);

			if ( iterations != 0)
				result[y*params->pixel_width+x] = iterations;
			else
				result[y*params->pixel_width+x] = 0;
		}
	}
}

#define MAX_BUFFERS 2

void show_fractal_on_screen(int *fractal, int fractal_width, int fractal_height) {
	gcmContextData *context;
	void *host_addr = NULL;
	rsxBuffer buffers[MAX_BUFFERS];
	int current_buffer = 0;
	padInfo padinfo;
	padData paddata;
	u16 width;
	u16 height;
	int i;

	printf("Running Mandel video test!\n");

	/* Allocate a 1Mb buffer, aligned to a 1Mb boundary
	* to be our shared IO memory with the RSX. */
	host_addr = memalign (1024*1024, HOST_SIZE);
	context = initScreen (host_addr, HOST_SIZE);
	ioPadInit(7);

	getResolution(&width, &height);
	for (i = 0; i < MAX_BUFFERS; i++)
		makeBuffer( &buffers[i], width, height, i);

	printf("Resolution: %i x %i\n", width, height);
	printf("Video init done.\n");

	flip(context, MAX_BUFFERS - 1);
	Palette palette = Palette();
	RSXPlotter plotter = RSXPlotter(&buffers[current_buffer], palette);
	waitFlip();

	// plot fractal here
	for (int y = 0; y<fractal_height; y++)
		for (int x = 0; x<fractal_width; x++) {
			int fractal_value = (int) fractal[(y*fractal_width)+x];
			int value = fractal_value == 0 ? 0 : 1;
			plotter.plot(x, y, value);
		}

	flip(context, buffers[current_buffer].id);
	waitFlip();

	while(1) {
		// check for user input for exit
		ioPadGetInfo(&padinfo);
		for(i=0; i<MAX_PADS; i++) {
			if(padinfo.status[i]) {
				ioPadGetData(i, &paddata);

				if(paddata.BTN_START)
					goto end;
			}
		}
	}

	end:

	gcmSetWaitFlip(context);
	for (i = 0; i < MAX_BUFFERS; i++)
		rsxFree(buffers[i].ptr);

	rsxFinish(context, 1);
	free(host_addr);
	ioPadEnd();
}
