#pragma once
#include "Entity.h"
#include <memory>
#include "OpenGL/Camera.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "World/ChunkManager.h"

enum class MovementInput {
	Forward,
	Left,
	Right,
	Back,
	Jump,
	Crouch,
	Sprint
};

enum class MovementType {
	Idle,
	Walking,
	Sneaking,
	Running
};

enum class GroundState {
	Grounded,
	Falling,
	Jumping,
	Flying
};

class Player : public Entity {
//TODO: Move position update logic from camera class to here, in update function. only use keyboard input function to set direction.  Update velocity in update, update entity position in physics-manager
public:
	Player(const glm::vec3& position);
	void SetPosition(const glm::vec3& position) override;
	glm::mat4 GetView() {
		return _camera->GetViewMatrix();
	}
	void Update(double delta) override;
	void ApplyMovement(const glm::vec2& direction, float maxSpeed, double delta);
	void ApplyGravity(float maxFallSpeed, float gravity, double delta);
	void HandleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);
	glm::vec2 GetDirection();
	void UpdateMovement();
	void ProcessMouseMovement(int xoffset, int yoffset);
	void ProcessMouseInput(GLFWwindow* window, int button, int action, int mods, ChunkManager* chunkManager, const glm::vec3& dir);
	
private:
	const float _acceleration = 40.0f;
	const float _gravity = 45.0f;
	const float _maxFallSpeed = -80.0f;
	const float _jumpSpeed = 12.0f;
	glm::vec2 _localVelocity = glm::vec2(0.0f);
	glm::vec2 _cameraRelativeSpeed = glm::vec2(0.0f);
	std::unordered_map<MovementInput, bool> _activeInputs;
	std::unordered_map<int, MovementInput> _inputs = {
		{GLFW_KEY_W, MovementInput::Forward},
		{GLFW_KEY_A, MovementInput::Left},
		{GLFW_KEY_D, MovementInput::Right},
		{GLFW_KEY_S, MovementInput::Back},
		{GLFW_KEY_SPACE, MovementInput::Jump},
		{GLFW_KEY_LEFT_SHIFT, MovementInput::Crouch},
		{GLFW_KEY_LEFT_CONTROL, MovementInput::Sprint}
	};
	MovementType _currentMovementType = MovementType::Idle;
	std::unordered_map<MovementType, float> _movementSpeeds = {
		{MovementType::Walking, 4.0f},
		{MovementType::Running, 6.5f},
		{MovementType::Sneaking, 2.5f}
	};
	GroundState _currentGroundState = GroundState::Grounded;
	std::unique_ptr<Camera> _camera;
};