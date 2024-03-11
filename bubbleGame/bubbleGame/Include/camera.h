#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	float velocity;
	Camera();
	Camera(glm::vec3 pos_vec, float vel);
};