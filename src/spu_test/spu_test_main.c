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

const int WIDTH = 1920;
const int HEIGHT = 1080;
const unsigned int MAX_ITERATIONS = 256;

#define ptr2ea(x)			((u64)((void*)(x)))

void *allocate_result_buffer(struct fractal_params *params);
void calculate_with_spus(void *result, struct fractal_params *params);
void show_fractal_on_screen(int *result, int width, int height);

int main(int argc, char* argv[]) {

	double start_real = -0.743643887037158704752191506114774;
	double start_imag = 0.131825904205311970493132056385139;
	double x_aspect = 16;
	double y_aspect = 9;
	double offset_real = x_aspect * 0.1;
	double offset_imag = y_aspect * 0.1;

	double min_re, max_re, min_im, max_im;
	min_re = start_real-offset_real;
	max_re = start_real+offset_real;
	min_im = start_imag-offset_imag;
	max_im = start_imag+offset_imag;

	printf("\n\nSPU CODE with full-size fractal:\n");
	double x_step = (max_re - min_re) / WIDTH;
	double y_step = (max_im - min_im) / HEIGHT;

	static struct fractal_params params __attribute__((aligned(128)));
	params.pixel_width = WIDTH;
	params.pixel_height = HEIGHT;
	params.min_re = min_re;
	params.max_im = max_im;
	params.x_step = x_step;
	params.y_step = y_step;
	params.max_iterations = MAX_ITERATIONS;

	void *big_result = allocate_result_buffer(&params);
	calculate_with_spus(big_result, &params);
	printf("Calculated %i number of pixels...", WIDTH*HEIGHT);
	show_fractal_on_screen((int *)big_result, WIDTH, HEIGHT);
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

	sysSpuThreadGroupStart(group_id);
	sysSpuThreadGroupJoin(group_id, &cause, &status);

	sysSpuThreadGroupDestroy(group_id);
	sysSpuImageClose(&image);
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
