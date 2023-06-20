#stage : vertex
#version 450 core

#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec3 InVertexPos;
layout(location = 1) in vec3 InVertexNormal;
layout(location = 2) in vec2 InVertexUV;

layout(push_constant, scalar) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Transform;
} InPushConstants;

layout(location = 0) out vec3 OutNormal;

void main()
{
    gl_Position = InPushConstants.ViewProjection * InPushConstants.Transform * vec4(InVertexPos, 1.0);
    OutNormal = InVertexNormal;
}

#stage : fragment
#version 450 core

layout(location = 0) in vec3 InNormal;

layout(location = 0) out vec4 OutColor;

void main()
{
    OutColor = vec4(InNormal * 0.5 + 0.5, 1.0);
}
