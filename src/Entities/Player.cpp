#include "Player.h"

Player::Player(const glm::vec3& position) {
	this->position = position;
	this->Shape = std::make_unique<CollisionShape>(glm::vec3(0.6f, 0.6f, 1.8f), glm::vec3(0.0f, 0.0f, 0.9f));
	//this->CollidesWithVoxels = false;
	_camera = std::make_unique<Camera>(position + glm::vec3(0.0f, 0.0f, 1.8f));
}
void Player::SetPosition(const glm::vec3& position) {
	_camera->Position = position + (_currentMovementType == MovementType::Sneaking ? glm::vec3(0.0f, 0.0f, 1.1f) : glm::vec3(0.0f, 0.0f, 1.5f));
	this->position = position;
}
void Player::Update(double delta) {
	_localVelocity.x = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Right));
	_localVelocity.y = glm::dot(glm::vec2(Velocity), glm::vec2(_camera->Forward));
	UpdateMovement();
	glm::vec2 direction = GetDirection();

	ApplyMovement(direction, _movementSpeeds[_currentMovementType], delta);

	ApplyGravity(_maxFallSpeed, _gravity, delta);
	//Jump
	if (_currentGroundState == GroundState::Jumping) {
		Velocity.z = _jumpSpeed;
	}
}
void Player::ApplyMovement(const glm::vec2& direction, float maxSpeed, double delta) {
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
void Player::ApplyGravity(float maxFallSpeed, float gravity, double delta) {
	//Gravity
	if (IsOnFloor)
		Velocity.z = 0.0f;
	Velocity.z -= gravity * delta;

	if (Velocity.z < maxFallSpeed)
		Velocity.z = maxFallSpeed;
}
void Player::HandleKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (!_inputs.count(key) || action == GLFW_REPEAT)
		return;
	_activeInputs[_inputs[key]] = (action == GLFW_PRESS);
}
glm::vec2 Player::GetDirection() {
	glm::vec2 direction = glm::vec2(0.0f);
	if (_activeInputs[MovementInput::Forward]) {
		direction.y += 1.0f;
	}
	if (_activeInputs[MovementInput::Back]) {
		direction.y -= 1.0f;
	}
	if (_activeInputs[MovementInput::Right]) {
		direction.x += 1.0f;
	}
	if (_activeInputs[MovementInput::Left]) {
		direction.x -= 1.0f;
	}
	if (direction != glm::vec2(0.0f))
		direction = glm::normalize(direction);

	return direction;
}
void Player::UpdateMovement() {
	switch (_currentMovementType) {
	case MovementType::Idle:
		if (_activeInputs[MovementInput::Forward] || _activeInputs[MovementInput::Back] || _activeInputs[MovementInput::Left] || _activeInputs[MovementInput::Right])
			_currentMovementType = MovementType::Walking;
		if (_activeInputs[MovementInput::Crouch])
			_currentMovementType = MovementType::Sneaking;
		break;
	case MovementType::Walking:
		if (!(_activeInputs[MovementInput::Forward] || _activeInputs[MovementInput::Back] || _activeInputs[MovementInput::Left] || _activeInputs[MovementInput::Right]))
			_currentMovementType = MovementType::Idle;
		if (_activeInputs[MovementInput::Sprint])
			_currentMovementType = MovementType::Running;
		else if (_activeInputs[MovementInput::Crouch])
			_currentMovementType = MovementType::Sneaking;
		break;
	case MovementType::Running:
		if (!_activeInputs[MovementInput::Sprint])
			_currentMovementType = MovementType::Walking;
		if (!(_activeInputs[MovementInput::Forward] || _activeInputs[MovementInput::Back] || _activeInputs[MovementInput::Left] || _activeInputs[MovementInput::Right]))
			_currentMovementType = MovementType::Idle;
		break;
	case MovementType::Sneaking:
		if (!_activeInputs[MovementInput::Crouch])
			_currentMovementType = MovementType::Idle;
		break;
	}
	switch (_currentGroundState) {
	case GroundState::Grounded:
		if (Velocity.z > 0)
			_currentGroundState = GroundState::Falling;
		else if (_activeInputs[MovementInput::Jump])
			_currentGroundState = GroundState::Jumping;
		break;
	case GroundState::Falling:
		if (IsOnFloor)
			_currentGroundState = GroundState::Grounded;
		break;
	case GroundState::Jumping:
		if (!IsOnFloor)
			_currentGroundState = GroundState::Falling;
		break;
	}
}
void Player::ProcessMouseMovement(int xoffset, int yoffset) {
	_camera->ProcessMouseMovement(xoffset, yoffset);
}
void Player::ProcessMouseInput(GLFWwindow* window, int button, int action, int mods, ChunkManager* chunkManager, const glm::vec3& dir) {
	int range = 10;
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		glm::vec3 origin = _camera->Position;
		glm::ivec3 voxel = glm::ivec3(glm::floor(origin));
		glm::ivec3 step = glm::ivec3(
			dir.x > 0 ? 1 : -1,
			dir.y > 0 ? 1 : -1,
			dir.z > 0 ? 1 : -1
		);
		glm::ivec3 nextBoundary = glm::ivec3(
			step.x > 0 ? voxel.x + 1 : voxel.x,
			step.y > 0 ? voxel.y + 1 : voxel.y,
			step.z > 0 ? voxel.z + 1 : voxel.z
		);
		glm::vec3 tDelta = glm::vec3(
			dir.x == 0 ? FLT_MAX : glm::abs(1.0f / dir.x),
			dir.y == 0 ? FLT_MAX : glm::abs(1.0f / dir.y),
			dir.z == 0 ? FLT_MAX : glm::abs(1.0f / dir.z)
		);

		glm::vec3 tMax = glm::vec3(
			dir.x == 0 ? FLT_MAX : (nextBoundary.x - origin.x) / dir.x,
			dir.y == 0 ? FLT_MAX : (nextBoundary.y - origin.y) / dir.y,
			dir.z == 0 ? FLT_MAX : (nextBoundary.z - origin.z) / dir.z
		);
		for (int i = 0; i < range; i++) {
			
			if (chunkManager->TryBreakBlock(voxel, true))
				break;
			if (tMax.x < tMax.y) {
				if (tMax.x < tMax.z) {
					voxel.x += step.x;
					tMax.x += tDelta.x;
				}
				else {
					voxel.z += step.z;
					tMax.z += tDelta.z;
				}
			}
			else {
				if (tMax.y < tMax.z) {
					voxel.y += step.y;
					tMax.y += tDelta.y;
				}
				else {
					voxel.z += step.z;
					tMax.z += tDelta.z;
				}
			}
		}
	}
}