#version 330 core
out vec4 FragColor;

in vec2 gTexture;
in float gAlpha;

uniform sampler2D ourTexture;

void main()
{
    vec4 FinalTex = texture(ourTexture, gTexture);
    FragColor = vec4(FinalTex.rgb, FinalTex.a * gAlpha);
}
