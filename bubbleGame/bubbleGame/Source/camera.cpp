#include "camera.h"

Camera::Camera() {};
Camera::Camera(glm::vec3 pos_vec,float vel) {
	position = pos_vec;
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	velocity = vel;
}