#pragma once
#include "Entity.h"
#include <memory>
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <unordered_map>

enum MovementInput {
	Forward,
	Left,
	Right,
	Back,
	Jump,
	Crouch,
	Sprint
};

enum MovementType {
	Idle,
	Walking,
	Sneaking,
	Running
};

enum GroundState {
	Grounded,
	Falling,
	Jumping,
	Flying
};

class Player : public Entity {
//TODO: Move position update logic from camera class to here, in update function. only use keyboard input function to set direction.  Update velocity in update, update entity position in physics-manager
public:
	Player(const glm::vec3& position) {
		this->position = position;
		this->Shape = std::make_unique<CollisionShape>(glm::vec3(0.6f, 0.6f, 1.8f), glm::vec3(0.0f, 0.0f, 0.9f));
		_camera = std::make_unique<Camera>(position + glm::vec3(0.0f, 0.0f, 1.8f));
	}
	void SetPosition(const glm::vec3& position) override {
		_camera->Position = position + (_currentMovementType == Sneaking ? glm::vec3(0.0f, 0.0f, 1.1f) : glm::vec3(0.0f, 0.0f, 1.5f));
		this->position = position;
	}
	glm::mat4 GetView() {
		return _camera->GetViewMatrix();
	}
	void Update(double delta) override {
		_localVelocity.x = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Right));
		_localVelocity.y = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Forward));
		UpdateMovement();
		glm::vec2 direction = GetDirection();
		
		ApplyMovement(direction, _movementSpeeds[_currentMovementType], delta);
		
		ApplyGravity(_maxFallSpeed, _gravity, delta);
		//Jump
		if (_currentGroundState == Jumping) {
			Velocity.z = _jumpSpeed;
		}
	}
	void ApplyMovement(const glm::vec2& direction, float maxSpeed, double delta) {
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

		if (glm::length(_localVelocity) > maxSpeed) {
			_localVelocity = maxSpeed * glm::normalize(_localVelocity);
		}

		Velocity = glm::vec3(glm::vec2(_localVelocity.x * _camera->Right + _localVelocity.y * _camera->Forward), Velocity.z);
	}
	void ApplyGravity(float maxFallSpeed, float gravity, double delta) {
		//Gravity
		if (IsOnFloor) 
			Velocity.z = 0.0f;
		Velocity.z -= gravity * delta;

		if (Velocity.z < maxFallSpeed)
			Velocity.z = maxFallSpeed;
	}
	void HandleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (!_inputs.count(key) || action == GLFW_REPEAT)
			return;
		_activeInputs[_inputs[key]] = (action == GLFW_PRESS);
	}
	glm::vec2 GetDirection() {
		glm::vec2 direction = glm::vec2(0.0f);
		if (_activeInputs[Forward]) {
			direction.y += 1.0f;
		}
		if (_activeInputs[Back]) {
			direction.y -= 1.0f;
		}
		if (_activeInputs[Right]) {
			direction.x += 1.0f;
		}
		if (_activeInputs[Left]) {
			direction.x -= 1.0f;
		}
		if (direction != glm::vec2(0.0f))
			direction = glm::normalize(direction);

		return direction;
	}
	void UpdateMovement() {
		switch (_currentMovementType) {
		case Idle:
			if (_activeInputs[Forward] || _activeInputs[Back] || _activeInputs[Left] || _activeInputs[Right])
				_currentMovementType = Walking;
			if (_activeInputs[Crouch])
				_currentMovementType = Sneaking;
			break;
		case Walking: 
			if (!(_activeInputs[Forward] || _activeInputs[Back] || _activeInputs[Left] || _activeInputs[Right]))
				_currentMovementType = Idle;
			if (_activeInputs[Sprint])
				_currentMovementType = Running;
			else if (_activeInputs[Crouch])
				_currentMovementType = Sneaking;
			break;
		case Running:
			if (!_activeInputs[Sprint])
				_currentMovementType = Walking;
			if (!(_activeInputs[Forward] || _activeInputs[Back] || _activeInputs[Left] || _activeInputs[Right]))
				_currentMovementType = Idle;
			break;
		case Sneaking:
			if (!_activeInputs[Crouch])
				_currentMovementType = Idle;
			break;
		}
		switch (_currentGroundState) {
		case Grounded:
			if (Velocity.z > 0)
				_currentGroundState = Falling;
			else if (_activeInputs[Jump])
				_currentGroundState = Jumping;
			break;
		case Falling:
			if (IsOnFloor)
				_currentGroundState = Grounded;
			break;
		case Jumping:
			if (!IsOnFloor)
				_currentGroundState = Falling;
			break;
		}
	}
	void ProcessMouseMovement(int xoffset, int yoffset) {
		_camera->ProcessMouseMovement(xoffset, yoffset);
	}
	
private:
	const float _acceleration = 40.0f;
	const float _gravity = 45.0f;
	const float _maxFallSpeed = -80.0f;
	const float _jumpSpeed = 12.0f;
	glm::vec2 _localVelocity = glm::vec2(0.0f);
	glm::vec2 _cameraRelativeSpeed = glm::vec2(0.0f);
	std::unordered_map<MovementInput, bool> _activeInputs;
	std::unordered_map<int, MovementInput> _inputs = {
		{GLFW_KEY_W, Forward},
		{GLFW_KEY_A, Left},
		{GLFW_KEY_D, Right},
		{GLFW_KEY_S, Back},
		{GLFW_KEY_SPACE, Jump},
		{GLFW_KEY_LEFT_SHIFT, Crouch},
		{GLFW_KEY_LEFT_CONTROL, Sprint}
	};
	MovementType _currentMovementType = Idle;
	std::unordered_map<MovementType, float> _movementSpeeds = {
		{Walking, 4.0f},
		{Running, 6.5f},
		{Sneaking, 2.5f}
	};
	GroundState _currentGroundState = Grounded;
	std::unique_ptr<Camera> _camera;
};