#include "character.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

Character::Character() {};
Character::Character(glm::vec3 pos, float vel, glm::vec3 scl) {
	position = pos;
	velocity = vel;
	scale = scl;
}