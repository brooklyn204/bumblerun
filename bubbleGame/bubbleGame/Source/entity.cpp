#include "entity.h"

bool Entity::collided(Entity other) {
	float thisminx = ((-0.5f) * scale.x) + position.x;
	float otherminx = ((-0.5f) * other.scale.x) + other.position.x;
	float thismaxx = ((0.5f) * scale.x) + position.x;
	float othermaxx = ((0.5f) * other.scale.x) + other.position.x;

	float thisminy = ((-0.5f) * scale.y) + position.y;
	float otherminy = ((-0.5f) * other.scale.y) + other.position.y;
	float thismaxy = ((0.5f) * scale.y) + position.y;
	float othermaxy = ((0.5f) * other.scale.y) + other.position.y;

	float thisminz = ((-0.5f) * scale.z) + position.z;
	float otherminz = ((-0.5f) * other.scale.z) + other.position.z;
	float thismaxz = ((0.5f) * scale.z) + position.z;
	float othermaxz = ((0.5f) * other.scale.z) + other.position.z;

	bool xOverlap = (thismaxx >= otherminx) && (othermaxx >= thisminx);
	bool yOverlap = (thismaxy >= otherminy) && (othermaxy >= thisminy);
	bool zOverlap = (thismaxz >= otherminz) && (othermaxz >= thisminz);

	return xOverlap && yOverlap && zOverlap;
}