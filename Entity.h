#pragma once
#include <glm/glm.hpp>
#include "CollisionShape.h"

class Entity {
protected:
	glm::vec3 position = glm::vec3(0.0f);
public:
	std::unique_ptr<CollisionShape> Shape;
	glm::vec3 Velocity = glm::vec3(0.0f);
	virtual void Update(double delta) { }
	virtual void Render() { }
	virtual void SetPosition(const glm::vec3& position) {
		this->position = position;
	}
	bool IsOnFloor = false;
	bool CollisionX = false;
	bool CollisionY = false;
	const glm::vec3& GetPosition() {
		return position;
	}
};