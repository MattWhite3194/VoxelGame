#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 CameraPos;
uniform sampler2D TextureAtlas;
uniform vec3 globalLightDirection = vec3(0.9, 0.8, 1.0);
uniform vec3 globalLightOpposite = vec3(-0.8, -0.7, -0.2);
uniform float fadeStartDistance;

void main()
{
    //vec2 uv = TexCoord;
    //float pad = 1.0 / 256.0;  // e.g. 1 / 256
    //uv = uv * (1.0 - 2.0 * pad) + pad;
    float distanceToPlayer = distance(CameraPos, FragPos);
    float brightness = max(dot(normalize(globalLightDirection), normalize(Normal)), dot(normalize(globalLightOpposite), normalize(Normal)) * 0.7);
    float fade = 1 - max(0, (distanceToPlayer - fadeStartDistance) / 10);
    FragColor = vec4(texture(TextureAtlas, TexCoord).rgb * brightness, 1.0) * fade;
}