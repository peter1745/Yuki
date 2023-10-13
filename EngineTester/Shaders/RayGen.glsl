#stage RTRayGen
#version 460
#extension GL_GOOGLE_include_directive : require

#include "RayCommon.glsl"

layout(set = 0, binding = 0) uniform image2D Image;

layout(location = 0) rayPayloadEXT vec3 Color;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec3 focalPoint = PC.cameraZ * cross(PC.cameraX, PC.cameraY);
	d.x *= float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

	vec3 origin = PC.cameraPos;
	vec3 direction = normalize((PC.cameraY * d.y * -1) + (PC.cameraX * d.x) - focalPoint);

	traceRayEXT(
		accelerationStructureEXT(PC.tlas),
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

	imageStore(Image, ivec2(gl_LaunchIDEXT.xy), vec4(Color, 1.0));
}