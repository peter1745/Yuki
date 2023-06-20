#pragma once

#include "Yuki/Math/Vec3.hpp"
#include "Yuki/Math/Vec2.hpp"

#include "Yuki/Rendering/RHI/RenderContext.hpp"

namespace Yuki {

	struct Vertex
	{
		Math::Vec3 Position;
		Math::Vec3 Normal;
		Math::Vec2 UV;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;
	};

	struct Mesh
	{
		Buffer* VertexBuffer;
		Buffer* IndexBuffer;
		uint32_t IndexCount;

		static Mesh FromMeshData(RenderContext* InRenderContext, const MeshData& InMeshData);
	};

}
