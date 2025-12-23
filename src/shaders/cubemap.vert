#version 330 core

// TODO: Implement CubeMap shading

layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord = aPos;

    mat4 rotView = mat4(mat3(view));
    vec4 position = vec4(aPos, 1.0f);
    vec4 pos = projection * rotView * position;

    gl_Position = pos.xyww;
}  
