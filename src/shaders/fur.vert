#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 Texture;
} vs_out;

uniform mat4 model;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    vs_out.WorldPos = world.xyz;

    mat3 normalMat = mat3(transpose(inverse(model)));
    vs_out.WorldNormal = normalize(normalMat * aNormal);

    vs_out.Texture = aTexCoord;
    gl_Position = world;
}
