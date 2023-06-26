#stage : vertex
#version 450 core

#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec3 InVertexPos;
layout(location = 1) in vec3 InVertexNormal;
layout(location = 2) in vec2 InVertexUV;
layout(location = 3) in uint InVertexMaterial;

/*layout(push_constant, scalar) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Transform;
} InPushConstants;*/

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
