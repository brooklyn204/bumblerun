#pragma once
#include "entity.h"

class Character : public Entity {
public:
	float velocity;
	Character();
	Character(glm::vec3 pos, float vel, glm::vec3 scl);
};