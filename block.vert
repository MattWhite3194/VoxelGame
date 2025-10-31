#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in uint faceIndex;
layout (location = 2) in uint texIndex;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const vec3 faceNormals[6] = vec3[](
    vec3( 0.0,  1.0,  0.0), // +Y → Front face
    vec3( 0.0, -1.0,  0.0), // -Y → Back face
    vec3(-1.0,  0.0,  0.0), // -X → Left face
    vec3( 1.0,  0.0,  0.0), // +X → Right face
    vec3( 0.0,  0.0, -1.0), // -Z → Bottom face
    vec3( 0.0,  0.0,  1.0)  // +Z → Top face
);

const vec2 texCoords[6] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0625, 1.0),
    vec2(0.0, 1.0 - 0.0625),
    vec2(0.0, 1.0 - 0.0625),
    vec2(0.0625, 1.0),
    vec2(0.0625, 1.0 - 0.0625)
);

//Texture coords - Bottom-left -> top-right
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = faceNormals[faceIndex];
    TexCoord = texCoords[texIndex];
}  