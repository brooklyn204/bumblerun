#include "obstacle.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

Obstacle::Obstacle() {};
Obstacle::Obstacle(glm::vec3 pos, glm::vec3 scl) {
	position = pos;
	scale = scl;
}