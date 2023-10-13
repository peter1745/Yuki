#extension GL_EXT_buffer_reference				: require
#extension GL_EXT_buffer_reference2				: require
#extension GL_EXT_buffer_reference_uvec2		: require
#extension GL_EXT_scalar_block_layout			: require
#extension GL_EXT_shader_image_load_formatted	: require
#extension GL_EXT_ray_tracing					: require
#extension GL_EXT_ray_tracing_position_fetch	: require

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer ShadingAttributes
{
	vec3 normal;
	vec2 texCoord;
	uint materialIndex;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Material
{
	uint baseColor;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Index
{
	uint value;
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer GeometryInfo
{
	ShadingAttributes shadingAttribs;
	Index indices;
};

layout(push_constant, scalar) uniform PushConstants
{
	uvec2 tlas;
	vec3 cameraPos;
	vec3 cameraX;
	vec3 cameraY;
	float cameraZ;
	GeometryInfo geometries;
	Material materials;
} PC;