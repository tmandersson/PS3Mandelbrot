// main.cpp
#include "sdlmandel.h"
#include <iostream>
using namespace std;

/*
int main(int argc, char *argv[])
{
	SDLMandel mand;

	mand.zoom(-2.0, 0.6, -1.3, 1.3);
	char test;
	cin >> test;
}*/

#include <qapplication.h>
#include "qtmandel.h"
const int Screenwidth = 500;
const int Screenheight = 500;
const int Maxiterations = 500;

int main(int argc, char *argv[])
{
   QApplication a( argc, argv );
   QTMandel w(0, "Mandel", Maxiterations, Screenwidth, Screenheight);
   a.setMainWidget(&w);
   w.show();
   return a.exec();
}
