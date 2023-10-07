#stage Vertex
#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Vertex
{
	vec3 Position;
};

layout(buffer_reference, scalar) buffer VertexData
{
	Vertex Data[];
};

layout(push_constant, scalar) uniform PushConstants
{
	uvec2 VertexBuffer;
	mat4 ViewProjection;
} InPushConstants;

void main()
{
	Vertex v = VertexData(InPushConstants.VertexBuffer).Data[gl_VertexIndex];
	gl_Position = InPushConstants.ViewProjection * vec4(v.Position, 1.0);
}

#stage Fragment
#version 460

layout(location = 0) out vec4 Color;

void main()
{
	Color = vec4(1.0, 0.0, 0.0, 1.0);
}
