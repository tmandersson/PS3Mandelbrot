#ifndef MANDEL_H_
#define MANDEL_H_

class IPlotter {
public:
	virtual void plot(int x, int y, int color) = 0;
};

class Mandel {
public:
	Mandel(int, int, IPlotter &);
	virtual ~Mandel();

	void zoom(double, double, double, double);
	void zoom_coord(int, int, int, int);  // zoom to pixel cords
	void zoom_back(); // zoom back one step

	void calculate_section(void *);
   
protected:
	int _width, _height;
	bool _have_painted; // Is set to True at start of plotting
	void paint(); // plot/calc the mandelbrot set

private:
	IPlotter &_plotter;
	double _old_min_re, _old_max_re, _old_min_im, _old_max_im;
	double _min_re, _max_re, _min_im, _max_im;
	unsigned int _max_iterations;

	double _x_step, _y_step;
	unsigned int *_results;
	void calculate_row(double, double, unsigned int *);
	unsigned int calculate(double, double);
};

#endif /* MANDEL_H_ */
