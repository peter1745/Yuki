#stage RTMiss
#version 460
#extension GL_GOOGLE_include_directive : require

#include "RayCommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 Color;

void main()
{
	Color = vec3(0.1, 0.1, 0.1);
}