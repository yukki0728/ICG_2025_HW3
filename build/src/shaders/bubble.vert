#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT
{
	vec3 worldPos;
	vec3 normal;
} vs_out;

uniform mat4 model;

void main()
{
	vec4 world = model * vec4(aPos, 1.0);
	vs_out.worldPos = world.xyz;
    vs_out.normal   = normalize(mat3(transpose(inverse(model))) * aNormal);
	gl_Position = world;
}
