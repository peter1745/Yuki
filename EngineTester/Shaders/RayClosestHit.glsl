#stage RTClosestHit
#version 460
#extension GL_GOOGLE_include_directive : require

#include "RayCommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 Color;

hitAttributeEXT vec2 BaryCentricCoords;

//#define DEBUG_NORMALS
//#define DEBUG_GEN_NORMALS
//#define DEBUG_UVS

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
	vec3 normal = attribs0.normal * w.x + attribs1.normal * w.y + attribs2.normal * w.z;
	
#if defined(DEBUG_NORMALS)
	#if defined(DEBUG_GEN_NORMALS)
		vec3 p0 = gl_HitTriangleVertexPositionsEXT[0];
		vec3 p1 = gl_HitTriangleVertexPositionsEXT[1];
		vec3 p2 = gl_HitTriangleVertexPositionsEXT[2];

		vec3 v0w = gl_ObjectToWorldEXT * vec4(p0, 1);
		vec3 v1w = gl_ObjectToWorldEXT * vec4(p1, 1);
		vec3 v2w = gl_ObjectToWorldEXT * vec4(p2, 1);

		vec3 v01 = v1w - v0w;
		vec3 v02 = v2w - v0w;

		normal = normalize(cross(v01, v02));

		if (gl_HitKindEXT != gl_HitKindFrontFacingTriangleEXT)
		{
			normal *= -1.0;
		}
	#endif

	Color = normal * 0.5 + 0.5;
#elif defined(DEBUG_UVS)
	Color = vec3(mod(uv, 1.0), 0.0);
#else
	Material m0 = PC.materials[geometry.materialIndex];
	vec4 baseColor = unpackUnorm4x8(m0.baseColor);

	int baseColorTextureIndex = m0.baseColorTextureIndex;

	if (baseColorTextureIndex != -1)
	{
		vec4 textureColor = texture(sampler2D(SampledImages[nonuniformEXT(baseColorTextureIndex)], Samplers[PC.DefaultSamplerHandle]), uv);
		Color = vec3(textureColor.xyz);
	}
	else
	{
		Color = vec3(baseColor.xyz);
	}

#endif

}