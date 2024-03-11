#pragma once

#include <iostream>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class ShaderManager {
public:
	GLuint vshader;
	GLuint fshader;
	unsigned int program;
	void initEntityShaders();
	void initEndScreenShaders();
	GLuint loadShader(const char* filename, int type);
};