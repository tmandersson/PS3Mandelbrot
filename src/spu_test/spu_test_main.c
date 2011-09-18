#include <stdio.h>
#include <sys/spu.h>
#include "spu_bin.h"
#include <core/fractal_params.h>

const int WIDTH = 20;
const int HEIGHT = 20;
const unsigned int MAX_ITERATIONS = 256;

#define ptr2ea(x)			((u64)((void*)(x)))

void calculate_with_spu(int *result, struct fractal_params *params);
void calculate_fractal(int *result, struct fractal_params *params);
void print_values(int *result);

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

	printf("\n\nSPU CODE:\n");
	int spu_result[HEIGHT*WIDTH];
	calculate_with_spu(spu_result, &params);
	print_values(spu_result);

	printf("\n\nSPU CODE in 2 passes:\n");
	int spu_result2[HEIGHT*WIDTH];
	int chunk_size = HEIGHT/2;
	params.pixel_height = chunk_size;
	int offset;

	for (int i=0; i<HEIGHT*WIDTH; i++)
		spu_result2[i] = 1;

	offset = chunk_size * 0;
	params.max_im -= (y_step * offset);
	calculate_with_spu(&spu_result2[offset*params.pixel_width], &params);

	offset = chunk_size * 1;
	params.max_im -= (y_step * offset);
	calculate_with_spu(&spu_result2[offset*params.pixel_width], &params);

	print_values(spu_result2);

	printf("\n\nExiting!\n");
	return 0;
}

void calculate_with_spu(int *result, struct fractal_params *params) {
	for (int i=0; i<params->pixel_height*params->pixel_width; i++)
		result[i] = 1;

	int size = sizeof(int) * params->pixel_width * params->pixel_height;
	size = size + (size%16); // need to dma transfer full blocks of 16 bytes
	void *result_buffer = malloc(size);

	sysSpuImage image;
	u32 group_id, thread_id;
	u32 cause, status;
	int thread_count = 1;
	int priority = 100;
	sysSpuThreadGroupAttribute grpattr = { 7+1, ptr2ea("fractal"), 0, 0 };
	sysSpuThreadAttribute attr = { ptr2ea("f_thread"), 8+1, SPU_THREAD_ATTR_NONE };
	sysSpuThreadArgument arg = { 0, 0, 0, 0 };

	sysSpuInitialize(6, 0);
	sysSpuThreadGroupCreate(&group_id, thread_count, priority, &grpattr);
	sysSpuImageImport(&image, spu_bin, SPU_IMAGE_PROTECT);

	int index_in_group = 0;
	arg.arg0 = ptr2ea(result_buffer);
	arg.arg1 = ptr2ea(params);
	sysSpuThreadInitialize(&thread_id, group_id, index_in_group, &image, &attr, &arg);

	sysSpuThreadGroupStart(group_id);
	sysSpuThreadGroupJoin(group_id, &cause, &status);

	int *p = (int *) result_buffer;
	for (int i=0; i<params->pixel_width*params->pixel_height; i++)
			result[i] = (int) *(p+i);

	sysSpuThreadGroupDestroy(group_id);
	sysSpuImageClose(&image);
	free(result_buffer);
}

void print_values(int *result) {
	for (int y=0; y<HEIGHT; y++) {
		for (int x=0; x<WIDTH; x++)
		{
			int value = result[(y*WIDTH)+x];
			if (value == 0)
				printf("    ");
			else
				printf("%.3Xh", result[(y*WIDTH)+x]);
		}
		printf("\n");
	}
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

