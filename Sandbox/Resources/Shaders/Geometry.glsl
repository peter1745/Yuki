#stage : vertex
#version 450 core

layout(location = 0) in vec3 InVertexPos;
layout(location = 1) in vec3 InVertexNormal;
layout(location = 2) in vec2 InVertexUV;

void main()
{
    gl_Position = vec4(InVertexPos, 1.0);
}

#stage : fragment
#version 450 core

layout(location = 0) out vec4 OutColor;

void main()
{
    OutColor = vec4(0.2, 0.3, 0.8, 1.0);
}
