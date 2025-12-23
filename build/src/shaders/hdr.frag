#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hdrTexture;

void main()
{    
    vec3 hdr = texture(hdrTexture, TexCoord).rgb;
    FragColor = vec4(hdr, 1.0);
}
