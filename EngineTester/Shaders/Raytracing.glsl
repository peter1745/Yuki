#stage RTRayGen
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_image_load_formatted : require

layout(push_constant, scalar) uniform PushConstants
{
	uvec2 TopLevelAS;
	vec3 ViewPos;
	vec3 CameraX;
	vec3 CameraY;
	float CameraZOffset;
} InPushConstants;

layout(set = 0, binding = 0) uniform image2D Image;

struct HitPayload
{
	vec3 Value;
};

layout(location = 0) rayPayloadEXT vec3 Payload;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec3 focalPoint = InPushConstants.CameraZOffset * cross(InPushConstants.CameraX, InPushConstants.CameraY);
	d.x *= float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

	vec3 origin = InPushConstants.ViewPos;
	vec3 direction = normalize((InPushConstants.CameraY * d.y) + (InPushConstants.CameraX * d.x) - focalPoint);

	traceRayEXT(
		accelerationStructureEXT(InPushConstants.TopLevelAS),
		gl_RayFlagsOpaqueEXT,
		0xFF,
		0,
		0,
		0,
		origin,
		0.0,
		direction,
		100000.0,
		0
	);

	//imageStore(Image, ivec2(gl_LaunchIDEXT.xy), vec4(direction * 0.5 + 0.5, 1.0));
	imageStore(Image, ivec2(gl_LaunchIDEXT.xy), vec4(Payload, 1.0));
}

#stage RTMiss
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 HitValue;

void main()
{
	HitValue = vec3(0.1, 0.1, 0.1);
}

#stage RTClosestHit
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 HitValue;
hitAttributeEXT vec3 Attribs;

void main()
{
	HitValue = vec3(1.0, 0.0, 0.0);
}