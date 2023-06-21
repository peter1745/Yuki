#stage : vertex
#version 450 core

#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec3 InVertexPos;
layout(location = 1) in vec3 InVertexNormal;
layout(location = 2) in vec2 InVertexUV;
layout(location = 3) in uint InVertexMaterial;

layout(push_constant, scalar) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Transform;
} InPushConstants;

layout(location = 0) out vec3 OutNormal;
layout(location = 1) out vec2 OutUV;
layout(location = 2) out flat uint OutMaterialIndex;

void main()
{
    gl_Position = InPushConstants.ViewProjection * InPushConstants.Transform * vec4(InVertexPos, 1.0);
    OutNormal = InVertexNormal;
    OutUV = InVertexUV;
    OutMaterialIndex = InVertexMaterial;
}

#stage : fragment
#version 450 core

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 InNormal;
layout(location = 1) in vec2 InUV;
layout(location = 2) in flat uint InMaterialIndex;

struct Material
{
    uint AlbedoTextureIndex;
};

layout(set = 0, binding = 0) buffer MaterialsBuffer
{
    Material Materials[];
};
layout(set = 0, binding = 1) uniform sampler2D InAlbedoTextures[];

layout(location = 0) out vec4 OutColor;

void main()
{
    Material material = Materials[InMaterialIndex];

    OutColor = vec4(InNormal * 0.5 + 0.5, 1.0);
    OutColor = texture(InAlbedoTextures[nonuniformEXT(material.AlbedoTextureIndex)], InUV);
}
