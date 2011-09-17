#ifndef FRACTAL_PARAMS_H_
#define FRACTAL_PARAMS_H_

struct fractal_params {
	int pixel_width;
	int pixel_height;
	double min_re;
	double max_im;
	double x_step;
	double y_step;
	unsigned int max_iterations;
	int padding;
};

#endif /* FRACTAL_PARAMS_H_ */
