#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <sys/spu_thread.h>

void calculate_fractal(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step);

struct fractal_params {
	int pixel_width;
	int pixel_height;
	double min_re;
	double max_im;
	double x_step;
	double y_step;
	double padding;
};

int main(uint64_t dest_addr, uint64_t param_addr, uint64_t arg3, uint64_t arg4)
{
	struct fractal_params params;
	mfc_get(&params, (void*) param_addr, sizeof(struct fractal_params), 0, 0, 0);

	int pixel_width = 20;
	int pixel_height = 20;

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
	double x_step = (max_re - min_re) / pixel_width;
	double y_step = (max_im - min_im) / pixel_height;
	int result[pixel_width*pixel_height];

	calculate_fractal(result, pixel_width, pixel_height, min_re, max_im, x_step, y_step);

	int transfer_size = sizeof(int) * pixel_width * pixel_height;
	transfer_size = transfer_size + (transfer_size%16); // need to dma transfer full blocks of 16 bytes
	mfc_put(result, (void *) dest_addr, transfer_size, 0, 0, 0);

	spu_thread_exit(0);
	return 0;
}

// TODO: Share code with PPU side instead.
// TODO: Get number of iterations as parameter.
const unsigned int MAX_ITERATIONS = 256;
unsigned int calculate(double c_re, double c_im)
{
	double z_re, z_im;
	z_re = z_im = 0;
	unsigned int iterations;
	int infinity = 0;

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
			infinity = 1;
	}

	// if the iteration don't reach infinity then C is part of the	Mandel set
	if (!infinity)
		return 0;
	else
		return iterations;
}

void calculate_fractal(int *result, int pixel_width, int pixel_height, double min_re, double max_im, double x_step, double y_step)
{
	double re;
	double im = max_im;
	int y;
	for (y = 0; y < pixel_height; y++) {
		if (y > 0) {
			im -= y_step;
		}
		re = min_re; // start with the first pixel on the row

		int x;
		for (x = 0; x < pixel_width; x++) {
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
