#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;
//out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//Texture coords - Bottom-left -> top-right
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	//TexCoord = aTexCoord / 16.0 + vec2(0.0625 * 1, 15.0 / 16.0);
}  