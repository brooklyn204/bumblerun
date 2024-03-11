#include <iostream>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shaders.h"



void ShaderManager::initEntityShaders() {
	// Load shaders
	vshader = loadShader("Shaders/Entity/vshader.glsl", GL_VERTEX_SHADER);
	fshader = loadShader("Shaders/Entity/fshader.glsl", GL_FRAGMENT_SHADER);

	// Set up program and link shaders
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	// Clean up
	glDeleteShader(vshader);
	glDeleteShader(fshader);
}

void ShaderManager::initEndScreenShaders() {
	// Load shaders
	vshader = loadShader("Shaders/EndScreen/vshader.glsl", GL_VERTEX_SHADER);

	fshader = loadShader("Shaders/EndScreen/fshader.glsl", GL_FRAGMENT_SHADER);

	// Set up program and link shaders
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	// Clean up
	glDeleteShader(vshader);
	glDeleteShader(fshader);
}

// Shader loader, taken from slides: https://comunidadfom.com/pluginfile.php/57462/mod_resource/content/1/Our_First_OpenGL_App.pdf
GLuint ShaderManager::loadShader(const char *filename, int type) {
	std::ifstream file;
	file.open(filename, std::ios::in);
	file.seekg(0, std::ios::end);
	unsigned int fileLen = file.tellg();
	file.seekg(std::ios::beg);
	char* source = new char[fileLen + 1];
	int i = 0;
	while (file.good())
	{
		source[i] = file.get();
		if (!file.eof()) i++;
		else fileLen = i;
	}
	source[fileLen] = '\0';
	file.close();
	// Creation and compilation of the shaders
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}