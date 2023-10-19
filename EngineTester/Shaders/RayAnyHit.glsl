#stage RTAnyHit
#version 460
#extension GL_GOOGLE_include_directive : require

#include "RayCommon.glsl"

hitAttributeEXT vec2 BaryCentricCoords;

const float c_AlphaCutoff = 0.5;

void main()
{
	GeometryInfo geometry = PC.geometries[gl_InstanceCustomIndexEXT];

	uint i0 = geometry.indices[gl_PrimitiveID * 3 + 0].value;
	uint i1 = geometry.indices[gl_PrimitiveID * 3 + 1].value;
	uint i2 = geometry.indices[gl_PrimitiveID * 3 + 2].value;

	vec3 w = vec3(1.0 - BaryCentricCoords.x - BaryCentricCoords.y, BaryCentricCoords.x, BaryCentricCoords.y);

	ShadingAttributes attribs0 = geometry.shadingAttribs[i0];
	ShadingAttributes attribs1 = geometry.shadingAttribs[i1];
	ShadingAttributes attribs2 = geometry.shadingAttribs[i2];

	vec2 uv = attribs0.texCoord * w.x + attribs1.texCoord * w.y + attribs2.texCoord * w.z;

	Material m0 = PC.materials[attribs0.materialIndex];
	Material m1 = PC.materials[attribs1.materialIndex];
	Material m2 = PC.materials[attribs2.materialIndex];

	//int baseColorTextureIndex = m0.baseColorTextureIndex * w.x + m1.baseColorTextureIndex * w.y + m2.baseColorTextureIndex * w.z;
	int baseColorTextureIndex = m0.baseColorTextureIndex;

	if (baseColorTextureIndex != -1)
	{
		vec4 textureColor = texture(sampler2D(SampledImages[nonuniformEXT(baseColorTextureIndex)], Samplers[PC.DefaultSamplerHandle]), uv);
		
		if (textureColor.a < c_AlphaCutoff)
		{
			ignoreIntersectionEXT;
		}
	}

}