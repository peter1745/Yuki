#stage : vertex
#version 450 core

#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec3 UV;
	uint MaterialIndex;
};

layout(buffer_reference, scalar) buffer VertexData
{
	Vertex Data[];
};

layout(buffer_reference, scalar) buffer TransformData
{
	mat4 Data[];
};

struct Object
{
	uint64_t VertexVA;
	uint64_t MaterialVA;
	uint BaseTextureOffset;
};

layout(buffer_reference, scalar) buffer ObjectData
{
	Object Data[];
};

layout(push_constant, scalar) uniform PushConstants
{
	mat4 ViewProjection;
	uint64_t ObjectVA;
	uint64_t TransformVA;
} InPushConstants;

layout(location = 0) out vec3 OutNormal;
layout(location = 1) out vec3 OutUV;
layout(location = 2) out flat uint OutMaterialIndex;
layout(location = 3) out flat uint OutBaseTextureOffset;
layout(location = 4) out flat uint64_t OutMaterialVA;

void main()
{
	Object object = ObjectData(InPushConstants.ObjectVA).Data[gl_InstanceIndex];
	Vertex v = VertexData(object.VertexVA).Data[gl_VertexIndex];
	mat4 transform = TransformData(InPushConstants.TransformVA).Data[gl_InstanceIndex];

	gl_Position = InPushConstants.ViewProjection * transform * vec4(v.Position, 1.0);
	OutNormal = v.Normal;
	OutUV = v.UV;
	OutMaterialIndex = v.MaterialIndex;
	OutBaseTextureOffset = object.BaseTextureOffset;
	OutMaterialVA = object.MaterialVA;
}

#stage : fragment
#version 450 core

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(location = 0) in vec3 InNormal;
layout(location = 1) in vec3 InUV;
layout(location = 2) in flat uint InMaterialIndex;
layout(location = 3) in flat uint InBaseTextureOffset;
layout(location = 4) in flat uint64_t InMaterialVA;

struct Material
{
	int AlbedoTextureIndex;
	uint AlbedoColor;
};

layout(buffer_reference, scalar) buffer MaterialData
{
	Material Data[];
};

// TODO(Peter): Consider an alternative approach to passing textures (could this be done with a buffer_reference like materials?)
layout(set = 0, binding = 0) uniform sampler2D InAlbedoTextures[];

layout(location = 0) out vec4 OutColor;

void main()
{
	OutColor = vec4(InUV, 1.0);
	
	Material material = MaterialData(InMaterialVA).Data[InMaterialIndex];

    vec4 albedoColor = unpackUnorm4x8(material.AlbedoColor);
	//albedoColor = vec4(InUV.x, 0.0, 0.0, 1.0);

	if (material.AlbedoTextureIndex == -1)
	{
		OutColor = albedoColor;
	}
	else
	{
		vec4 colX = texture(InAlbedoTextures[nonuniformEXT(material.AlbedoTextureIndex)], InUV.zy);
		vec4 colY = texture(InAlbedoTextures[nonuniformEXT(material.AlbedoTextureIndex)], InUV.xz);
		vec4 colZ = texture(InAlbedoTextures[nonuniformEXT(material.AlbedoTextureIndex)], InUV.xy);

		vec3 blendWeight = abs(InNormal);
		float d = dot(blendWeight, vec3(1.0));
		blendWeight /= vec3(d, d, d);
		
		OutColor = colX * blendWeight.x + colY * blendWeight.y + colZ * blendWeight.z;
	}
}
