#pragma once
#include <glm/glm.hpp>

class CollisionShape {
public:
	CollisionShape(const glm::vec3& size, const glm::vec3& origin) : Size(size), Origin(origin) { }
	glm::vec3 Size;
	/// <summary>
	/// The origin point for the center of the collider, relative to the entities position
	/// </summary>
	glm::vec3 Origin;
	
};