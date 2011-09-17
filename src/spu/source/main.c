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

/* wait for dma transfer to be finished */
static void wait_for_completion(int tag) {
	mfc_write_tag_mask(1<<tag);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

int main(uint64_t dest_addr, uint64_t param_addr, uint64_t arg3, uint64_t arg4)
{
	struct fractal_params params;
	int tag = 1;
	mfc_get(&params, (uint32_t) param_addr, sizeof(struct fractal_params), tag, 0, 0);
	wait_for_completion(tag);

	int result[params.pixel_width * params.pixel_height];
	calculate_fractal(result, params.pixel_width, params.pixel_height, params.min_re, params.max_im, params.x_step, params.y_step);

	int transfer_size = sizeof(int) * params.pixel_width * params.pixel_height;
	transfer_size = transfer_size + (transfer_size%16); // need to dma transfer full blocks of 16 bytes
	mfc_put(result, (uint32_t) dest_addr, transfer_size, tag, 0, 0);
	wait_for_completion(tag);

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
