#stage : vertex
#version 450 core

#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 UV;
	uint MaterialIndex;
};

layout(buffer_reference, scalar) buffer VertexData
{
	Vertex Data[];
};

layout(push_constant, scalar) uniform PushConstants
{
	mat4 ViewProjection;
	mat4 Transform;
	uint64_t VertexVA;
} InPushConstants;

layout(location = 0) out vec3 OutNormal;
layout(location = 1) out vec2 OutUV;
layout(location = 2) out flat uint OutMaterialIndex;

void main()
{
	Vertex v = VertexData(InPushConstants.VertexVA).Data[gl_VertexIndex];

    gl_Position = InPushConstants.ViewProjection * InPushConstants.Transform * vec4(v.Position, 1.0);
    OutNormal = v.Normal;
    OutUV = v.UV;
    OutMaterialIndex = v.MaterialIndex;
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
	OutColor = vec4(InUV, 0.0, 1.0);
	OutColor = vec4(InNormal * 0.5 + 0.5, 1.0);
	
    Material material = Materials[InMaterialIndex];
    OutColor = texture(InAlbedoTextures[nonuniformEXT(material.AlbedoTextureIndex)], InUV);
}
