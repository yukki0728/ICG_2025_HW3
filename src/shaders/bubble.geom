#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT 
{
    vec3 worldPos;
    vec3 normal;
} gs_in[];