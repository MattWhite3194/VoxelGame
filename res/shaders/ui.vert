#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform mat4 Transform;
uniform mat4 Projection;

void main() {
	gl_Position = Projection * Transform * vec4(aPos, 0.0, 1.0);
	TexCoord = aTex;
}