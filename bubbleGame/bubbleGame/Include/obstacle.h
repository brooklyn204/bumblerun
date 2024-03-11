#pragma once
#include "entity.h"

class Obstacle : public Entity {
public:
	Obstacle();
	Obstacle(glm::vec3 pos, glm::vec3 scl);
};