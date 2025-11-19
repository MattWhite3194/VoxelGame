#include "UI/UIComponent.h"
#include <glm/gtx/transform.hpp>

UIComponent::UIComponent(std::shared_ptr<Texture> texture, const glm::vec2& position, const glm::vec2& size, Anchor anchor) : texture(texture), position(position), size(size), anchor(anchor) {
	transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
}

void UIComponent::UpdateAnchorMatrix(int parentWidth, int parentHeight) {
    transformMatrix = GetAnchorMatrix(parentWidth, parentHeight) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
}

glm::mat4 UIComponent::GetAnchorMatrix(int parentWidth, int parentHeight) {
    switch (anchor) {
    case Anchor::TopLeft:
        return glm::translate(glm::mat4(1.0f), glm::vec3(position.x,                                        parentHeight - size.y + position.y, 0.0f));
    case Anchor::TopMiddle:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth / 2.0f + position.x - size.x / 2.0f,   parentHeight - size.y + position.y, 0.0f));
    case Anchor::TopRight:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth - size.x + position.x,                 parentHeight - size.y + position.y, 0.0f));

    case Anchor::MiddleLeft:
        return glm::translate(glm::mat4(1.0f), glm::vec3(position.x,                                        parentHeight / 2.0f - size.y / 2.0f + position.y, 0.0f));
    case Anchor::MiddleMiddle:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth / 2.0f + position.x - size.x / 2.0f,   parentHeight / 2.0f - size.y / 2.0f + position.y, 0.0f));
    case Anchor::MiddleRight:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth - size.x + position.x,                 parentHeight / 2.0f - size.y / 2.0f + position.y, 0.0f));

    case Anchor::BottomLeft:
        return glm::translate(glm::mat4(1.0f), glm::vec3(position.x,                                        position.y, 0.0f));
    case Anchor::BottomMiddle:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth / 2.0f + position.x - size.x / 2.0f,   position.y, 0.0f));
    case Anchor::BottomRight:
        return glm::translate(glm::mat4(1.0f), glm::vec3(parentWidth - size.x + position.x,                 position.y, 0.0f));
    }
}