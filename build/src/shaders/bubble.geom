#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT 
{
    vec3 worldPos;
    vec3 normal;
} gs_in[];

out vec3 gNormal;
out vec3 gWorldPos;

uniform mat4 view;
uniform mat4 projection;

uniform float BubbleAmount;

void main()
{
    for(int i = 0; i < 3; i++)
    {
        vec3 N = normalize(gs_in[i].normal);
        gWorldPos = gs_in[i].worldPos + N * BubbleAmount * 5.0f;
        gNormal = N;

        gl_Position = projection * view * vec4(gWorldPos, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
