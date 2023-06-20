#pragma once

#include "Yuki/Math/Vec2.hpp"
#include "Yuki/Math/Mat4.hpp"

#include "Yuki/Rendering/RHI/RenderContext.hpp"

namespace Yuki {

	struct Vertex
	{
		Math::Vec3 Position;
		Math::Vec3 Normal;
		Math::Vec2 UV;
	};

	struct Mesh
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		Buffer* VertexBuffer;
		Buffer* IndexBuffer;
	};

	struct MeshInstance
	{
		Mesh* SourceMesh;
		Math::Mat4 Transform;
	};

	struct LoadedMesh
	{
		List<Mesh> Meshes;
		List<MeshInstance> Instances;
	};

}
