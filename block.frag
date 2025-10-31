#version 460 core

out vec4 FragColor;

in vec3 FragPos;
//in vec3 Normal;
//in vec2 TexCoord;

uniform sampler2D TextureAtlas;

void main()
{
    //FragColor = texture(TextureAtlas, TexCoord);
    FragColor = vec4(0.5, 0.5, 0.5, 1.0);
}