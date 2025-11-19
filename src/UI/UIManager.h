#pragma once
#include <vector>
#include "UI/UIComponent.h"
#include "OpenGL/Shader.h"

class UIManager {
public:
	void Initialize(int viewportWidth, int viewportHeight);
	void Update(Shader& shader, const glm::mat4& projection);
	void AddUIComponent(std::shared_ptr<UIComponent> component);
	void OnViewportResized(int viewportWidth, int viewportHeight);
private:
	GLuint VAO = 0, VBO = 0;
	int viewportWidth, viewportHeight;
	std::vector<std::shared_ptr<UIComponent>> _components;
	static constexpr float _vertices[24] = {
		//bl
		0.0f, 0.0f, 0.0f, 0.0f,
		//tl
		0.0f, 1.0f, 0.0f, 1.0f,
		//br
		1.0f, 0.0f, 1.0f, 0.0f,

		//br
		1.0f, 0.0f, 1.0f, 0.0f,
		//tl
		0.0f, 1.0f, 0.0f, 1.0f,
		//tr
		1.0f, 1.0f, 1.0f, 1.0f
	};
};