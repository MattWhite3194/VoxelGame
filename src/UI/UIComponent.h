#pragma once
#include "OpenGL/Texture.h"
#include <memory>
#include <glm/glm.hpp>
#include "Anchor.h"

class UIComponent {
public:
	std::shared_ptr<Texture> texture;
	glm::mat4 transformMatrix;
	glm::vec2 size;
	glm::vec2 position;
	Anchor anchor;

	UIComponent(std::shared_ptr<Texture> texture, const glm::vec2& position, const glm::vec2& size, Anchor anchor = Anchor::MiddleMiddle);
	void UpdateAnchorMatrix(int parentWidth, int parentHeight);
private:
	glm::mat4 GetAnchorMatrix(int parentWidth, int parentHeight);
};