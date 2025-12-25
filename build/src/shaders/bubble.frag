#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform float BubbleAmount;
uniform sampler2D ourTexture;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(viewPos - vWorldPos);

    if (3.0f * vWorldPos.y +  2.0f * vWorldPos.x >= -90.0f)
    {
        FragColor = texture(ourTexture, TexCoord);
    }
    else
    {
        float fresnel = pow(1.0 - dot(N, V), 3.0);
        vec3 bubbleColor = mix(vec3(0.7, 0.85, 1.0), vec3(1.0), fresnel);

        FragColor = vec4(bubbleColor, 0.15 + 0.35 * fresnel);
    }
} 
