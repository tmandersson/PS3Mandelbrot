#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <sys/spu_thread.h>
#include <core/fractal_params.h>

void calculate_fractal(int *result, struct fractal_params *params);

/* wait for dma transfer to be finished */
static void wait_for_completion(int tag) {
	mfc_write_tag_mask(1<<tag);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

int main(uint64_t dest_addr, uint64_t param_addr, uint64_t arg3, uint64_t arg4)
{
	static struct fractal_params params __attribute__((aligned(128)));
	int tag = 1;
	mfc_get(&params, (uint32_t) param_addr, sizeof(struct fractal_params), tag, 0, 0);
	wait_for_completion(tag);

	int transfer_size = sizeof(int) * params.pixel_width * params.pixel_height;
	int max_chunk = 16*1024;
	if (transfer_size > max_chunk)
		transfer_size += transfer_size % max_chunk;

	int result[transfer_size/sizeof(int)];
	calculate_fractal(result, &params);

	unsigned int offset = 0;
	if (transfer_size > max_chunk) { // need to dma transfer full 16kb chunks
		while (transfer_size > 0) {
			mfc_put(&result[offset], (uint32_t) (dest_addr + (offset*sizeof(int))), max_chunk, tag, 0, 0);
			transfer_size -= max_chunk;
			offset += max_chunk/sizeof(int);
		}
	}
	else {
		transfer_size += transfer_size % 16; // need to dma transfer full blocks of 16 bytes
		mfc_put(&result[offset], (uint32_t) (dest_addr + (offset*sizeof(int))), transfer_size, tag, 0, 0);
	}
	wait_for_completion(tag);

	spu_thread_exit(0);
	return 0;
}

// TODO: Share code with PPU side instead.
unsigned int calculate(double c_re, double c_im, unsigned int max_iterations)
{
	double z_re, z_im;
	z_re = z_im = 0;
	unsigned int iterations;
	int infinity = 0;

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
			infinity = 1;
	}

	// if the iteration don't reach infinity then C is part of the	Mandel set
	if (!infinity)
		return 0;
	else
		return iterations;
}

void calculate_fractal(int *result, struct fractal_params *params)
{
	double re;
	double im = params->max_im;
	int y;
	for (y = 0; y < params->pixel_height; y++) {
		if (y > 0) {
			im -= params->y_step;
		}
		re = params->min_re; // start with the first pixel on the row

		int x;
		for (x = 0; x < params->pixel_width; x++) {
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
