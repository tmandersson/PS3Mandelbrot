#include <stdio.h>

#ifdef __powerpc64__
#include <sys/spu.h>
#include "spu_bin.h"
void calculate_with_spu(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step);
#endif

const int WIDTH = 20;
const int HEIGHT = 20;

void calculate_fractal(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step);
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
	calculate_fractal(result, WIDTH, HEIGHT, min_re, max_im, x_step, y_step);
	print_values(result);

	printf("\n\nSPU CODE:\n");
#ifdef __powerpc64__
	int spu_result[HEIGHT*WIDTH];
	calculate_with_spu(spu_result, WIDTH, HEIGHT, min_re, max_im, x_step, y_step);
	print_values(spu_result);
#else
	printf("Not implemented in linux version.\n");
#endif

	printf("Sizeof int: %lu", sizeof(int));
	printf("Sizeof double: %lu", sizeof(double));
	printf("Sizeof void*: %lu", sizeof(void*));
	printf("\n\nExiting!\n");
	return 0;
}

#ifdef __powerpc64__
#define ptr2ea(x)			((u64)((void*)(x)))

void calculate_with_spu(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step) {
	for (int i=0; i<HEIGHT*WIDTH; i++)
		result[i] = 1;

	int size = sizeof(int) * pixel_width * pixel_height;
	size = size + (size%16); // need to dma transfer full blocks of 16 bytes
	void *buffer = malloc(size);

	sysSpuImage image;
	u32 group_id, thread_id;
	u32 cause, status;
	int thread_count = 1;
	int priority = 100;
	sysSpuThreadGroupAttribute grpattr = { 7+1, ptr2ea("fractal"), 0, 0 };
	sysSpuThreadAttribute attr = { ptr2ea("f_thread"), 8+1, SPU_THREAD_ATTR_NONE };
	sysSpuThreadArgument arg = { (u64)buffer, 0, 0, 0 };

	sysSpuInitialize(6, 0);
	sysSpuThreadGroupCreate(&group_id, thread_count, priority, &grpattr);
	sysSpuImageImport(&image, spu_bin, SPU_IMAGE_PROTECT);
	int index_in_group = 0;
	sysSpuThreadInitialize(&thread_id, group_id, index_in_group, &image, &attr, &arg);

	sysSpuThreadGroupStart(group_id);
	sysSpuThreadGroupJoin(group_id, &cause, &status);

	int *p = (int *)buffer;
	for (int i=0; i<HEIGHT*WIDTH; i++)
			result[i] = (int) *(p+i);

	sysSpuThreadGroupDestroy(group_id);
	sysSpuImageClose(&image);
	free(buffer);
}
#endif

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

const unsigned int MAX_ITERATIONS = 256;
unsigned int calculate(double c_re, double c_im)
{
	double z_re, z_im;
	z_re = z_im = 0;
	unsigned iterations;
	bool infinity = false;

	// Stop when we maximum number of iterations is reached (part of Mandel set)
	// or when we're certain that the iteration is going to reach infinity.
	for (iterations = 0; iterations < MAX_ITERATIONS && !infinity; iterations++) {
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

void calculate_fractal(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step) {
	double re;
	double im = max_im;
	for (int y = 0; y < pixel_height; y++) {
		if (y > 0) {
			im -= y_step;
		}
		re = min_re; // start with the first pixel on the row

		for (int x = 0; x < pixel_width; x++) {
			if (x > 0)
				re += x_step;

			unsigned int iterations = calculate(re, im);

			if ( iterations != 0)
				result[y*pixel_width+x] = iterations;
			else
				result[y*pixel_width+x] = 0;
		}
	}
}

