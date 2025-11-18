#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in uint faceIndex;
layout (location = 2) in uint texIndex;
layout (location = 3) in uint blockID;

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

//0.0001 padding to prevent texture bleeding on mipmaps
const vec2 texCoords[6] = vec2[](
    vec2(0.0001, 1.0 - 0.0625 + 0.0001),
    vec2(0.0625 - 0.0001, 1.0 - 0.0625 + 0.0001),
    vec2(0.0001, 1.0 - 0.0001),
    vec2(0.0001, 1.0 - 0.0001),
    vec2(0.0625 - 0.0001, 1.0 - 0.0625 + 0.0001),
    vec2(0.0625 - 0.0001, 1.0 - 0.0001)
);

const vec3 blockCoords[3] = vec3[](
        //top, side, bottom
    //stone
    vec3(1, 1, 1),
    //dirt
    vec3(2, 2, 2),
    //grass
    vec3(0, 3, 2)
);

//Texture coords - Bottom-left -> top-right
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = faceNormals[faceIndex];

    int index = 0;
    if (Normal.z < 0)
        index = 2;
    else if (abs(Normal.x) + abs(Normal.y) > 0)
        index = 1;
    //TODO: modulation for y axis for blockids that are greater or equal to 16, every 16 blocks, add 0.0625 to y
    TexCoord = texCoords[texIndex] + vec2(0.0625 * blockCoords[blockID - 1][index], 0.0);
}  