#pragma once
#include "Entity.h"
#include <memory>
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <unordered_set>

class Player : public Entity {
//TODO: Move position update logic from camera class to here, in update function. only use keyboard input function to set direction.  Update velocity in update, update entity position in physics-manager
public:
	Player(const glm::vec3& position) {
		this->position = position;
		this->Shape = std::make_unique<CollisionShape>(glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		_camera = std::make_unique<Camera>(position + glm::vec3(0.0f, 0.0f, 1.8f));
	}
	void SetPosition(const glm::vec3& position) override {
		_camera->Position = position + glm::vec3(0.0f, 0.0f, 1.5f);
		this->position = position;
	}
	glm::mat4 GetView() {
		return _camera->GetViewMatrix();
	}
	void Update(double delta) override {
		_localVelocity.x = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Right));
		_localVelocity.y = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Forward));
		glm::vec2 direction = glm::vec2(0.0f);
		if (_activeInputs.count(GLFW_KEY_W)) {
			direction.y += 1.0f;
		}
		if (_activeInputs.count(GLFW_KEY_S)) {
			direction.y -= 1.0f;
		}
		if (_activeInputs.count(GLFW_KEY_D)) {
			direction.x += 1.0f;
		}
		if (_activeInputs.count(GLFW_KEY_A)) {
			direction.x -= 1.0f;
		}
		if (direction != glm::vec2(0.0f))
			direction = glm::normalize(direction);
		if (direction.x != 0.0f) {
			_localVelocity.x += _acceleration * delta * direction.x;
		}
		else if (_localVelocity.x != 0.0f) {
			int sign = glm::sign<float>(_localVelocity.x);
			_localVelocity.x -= _acceleration * delta * sign;
			if (glm::sign<int>(_localVelocity.x) != sign)
				_localVelocity.x = 0.0f;
		}
		if (direction.y != 0.0f) {
			_localVelocity.y += _acceleration * delta * direction.y;
		}
		else if (_localVelocity.y != 0.0f) {
			int sign = glm::sign<float>(_localVelocity.y);
			_localVelocity.y -= _acceleration * delta * sign;
			if (glm::sign<int>(_localVelocity.y) != sign)
				_localVelocity.y = 0.0f;
		}
		
		if (glm::length(_localVelocity) > _maxMovementSpeed) {
			_localVelocity = _maxMovementSpeed * glm::normalize(_localVelocity);
		}

		Velocity = glm::vec3(glm::vec2(_localVelocity.x * _camera->Right + _localVelocity.y * _camera->Forward), Velocity.z);
		//Gravity
		if (IsOnFloor)
			Velocity.z = 0.0f;
		Velocity.z -= _gravity * delta;
		if (Velocity.z < _maxFallSpeed)
			Velocity.z = _maxFallSpeed;

		//Jump
		if (_jumping && IsOnFloor) {
			Velocity.z = _jumpSpeed;
		}
	}
	void HandleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_RELEASE) {
			if (_activeInputs.count(key)) {
				_activeInputs.erase(key);
			}
			if (key == GLFW_KEY_SPACE) {
				_jumping = false;
			}
		}
		else if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) {
				_activeInputs.insert(key);
			}
			if (key == GLFW_KEY_SPACE) {
				_jumping = true;
			}
		}
	}
	void ProcessMouseMovement(int xoffset, int yoffset) {
		_camera->ProcessMouseMovement(xoffset, yoffset);
	}
	
private:
	const float _acceleration = 40.0f;
	const float _maxMovementSpeed = 6.0f;
	const float _gravity = 45.0f;
	const float _maxFallSpeed = -80.0f;
	const float _jumpSpeed = 12.0f;
	bool _jumping = false;
	glm::vec2 _localVelocity = glm::vec2(0.0f);
	glm::vec2 _cameraRelativeSpeed = glm::vec2(0.0f);
	std::unordered_set<int> _activeInputs;
	std::unique_ptr<Camera> _camera;
};