// Main.cpp
#include "sdlmandel.h"
#include <iostream>
using namespace std;

const int WIDTH = 1280;
const int HEIGHT = 720;
const unsigned int ITERATIONS = 500;

int main(int argc, char *argv[])
{
  SDLMandel mand = SDLMandel(ITERATIONS, WIDTH, HEIGHT);

  mand.zoom(-3.0, 1.6, -1.3, 1.3);
  char test;
  cin >> test;
}
