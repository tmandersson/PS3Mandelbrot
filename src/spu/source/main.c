#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <sys/spu_thread.h>
#include <core/fractal_params.h>

void calculate_fractal();
void transfer_data();

// maximum data we can store/calculate
const int max_calculation_size = 112*1024;
// maximum data we can transfer with dma
const int max_transfer_size = 16*1024;

// wait for dma transfer to be finished
static void wait_for_completion(int tag) {
	mfc_write_tag_mask(1<<tag);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

int dma_tag = 1;

static struct fractal_params params __attribute__((aligned(128)));

int result[(240*1024)/sizeof(int)]; // 240 kb result buffer
int transfer_size = 0;
uint64_t destination;

int main(uint64_t dest_addr, uint64_t param_addr, uint64_t arg3, uint64_t arg4)
{
	mfc_get(&params, (uint32_t) param_addr, sizeof(struct fractal_params), dma_tag, 0, 0);
	wait_for_completion(dma_tag);

	destination = dest_addr;
	calculate_fractal();
	transfer_data();

	spu_thread_exit(0);
	return 0;
}

void transfer_data() {
	if (transfer_size == 0)
		return;

	if (transfer_size > max_transfer_size)
		transfer_size += transfer_size % max_transfer_size;

	unsigned int offset = 0;
	if (transfer_size > max_transfer_size) { // need to dma transfer full 16kb chunks
		while (transfer_size > 0) {
			mfc_put(&result[offset], (uint32_t) (destination + (offset*sizeof(int))), max_transfer_size, dma_tag, 0, 0);
			transfer_size -= max_transfer_size;
			offset += max_transfer_size/sizeof(int);
		}
	}
	else {
		transfer_size += transfer_size % 16; // need to dma transfer full blocks of 16 bytes
		mfc_put(&result[offset], (uint32_t) (destination + (offset*sizeof(int))), transfer_size, dma_tag, 0, 0);
	}
	wait_for_completion(dma_tag);
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

void calculate_fractal()
{
	double re;
	double im = params.max_im;
	int y;
	for (y = 0; y < params.pixel_height; y++) {
		if (y > 0) {
			im -= params.y_step;
		}
		re = params.min_re; // start with the first pixel on the row

		int x;
		for (x = 0; x < params.pixel_width; x++) {
			if (x > 0)
				re += params.x_step;

			unsigned int iterations = calculate(re, im, params.max_iterations);

			if ( iterations != 0)
				result[y*params.pixel_width+x] = iterations;
			else
				result[y*params.pixel_width+x] = 0;

			transfer_size += sizeof(int);
			if (transfer_size == max_calculation_size)
			{
				transfer_data();
				destination += max_calculation_size;
				transfer_size = 0;
			}
		}
	}
}
