#version 330 core

in vec3 TexCoord;

out vec4 FragColor;

uniform sampler2D equirectangularMap;

const float PI = 3.14159265359;

void main()
{
    vec3 dir = normalize(TexCoord);
    float phi = atan(dir.z, dir.x);
    float theta = asin(clamp(dir.y, -1.0, 1.0));
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI + 0.5);
    vec3 color = texture(equirectangularMap, uv).rgb;
    // simple Reinhard tonemapping + gamma
    color = vec3(1.0) - exp(-color);
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}
