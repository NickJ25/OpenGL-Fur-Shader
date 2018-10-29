// PNG Writer, base code by Guillaume Cottenceau (http://zarb.org/~gc/html/libpng.html)
// Working modification by Yoshimasa Niwa (https://gist.github.com/niw/5963798)
#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include<iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include<png.h>

using namespace std;

class PNGProcessor {
private:
	int width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;
public:
	void readPNG(const char* file_name);
	GLuint processPNG(float perlin_freq);
	void writePNG(const char* file_name);
};