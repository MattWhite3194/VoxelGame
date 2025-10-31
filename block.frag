#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D TextureAtlas;
uniform vec3 globalLightDirection = vec3(0.9, 0.8, 1.0);
uniform vec3 globalLightOpposite = vec3(-0.8, -0.7, -0.2);

void main()
{
    float brightness = max(dot(normalize(globalLightDirection), normalize(Normal)), dot(normalize(globalLightOpposite), normalize(Normal)) * 0.7);
    FragColor = vec4(texture(TextureAtlas, TexCoord).rgb * brightness, 1.0);
}