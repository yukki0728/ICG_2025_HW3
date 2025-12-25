#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 96) out;

in VS_OUT
{
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 Texture;
} gs_in[];

out vec2 gTexture;
out float gAlpha;

uniform mat4 view;
uniform mat4 projection;

uniform float Strength; 
uniform float Length; 

float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

int LAYERS = 32;

void main()
{
    //n = (n1 + n2 + n3)/3
    vec3 N = normalize(gs_in[0].WorldNormal + gs_in[1].WorldNormal + gs_in[2].WorldNormal);

    for (int layer = 0; layer < LAYERS; layer++)
    {
        float t = float(layer) / float(LAYERS - 1); //0~1
        gAlpha = 1.0 - t; //越外層越透明

        float Len = Length * t * Strength; //越外層越長

        for (int i = 0; i < 3; i++)
        {
            float random_value = (hash12(gs_in[i].Texture * 40.0 + t * 10.0) - 0.5) * 0.15; //-0.075~0.075

            float len = Len * (1.0 + random_value);
            if (gs_in[i].WorldPos.x * 2.0f + gs_in[i].WorldPos.y * 3.0f >= -160.0f) len = 0; //頭不要長

            gl_Position = projection * view * vec4(gs_in[i].WorldPos + N * len, 1.0);

            gTexture = gs_in[i].Texture;

            EmitVertex();
        }
        EndPrimitive();
    }
}