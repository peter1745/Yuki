#pragma once

#include "Yuki/Math/Vec2.hpp"
#include "Yuki/Math/Mat4.hpp"

#include "RHI.hpp"

namespace Yuki {

	struct Vertex
	{
		Math::Vec3 Position;
		Math::Vec3 Normal;
		Math::Vec2 UV;
		uint32_t MaterialIndex = 0;
	};

	struct MeshMaterial
	{
		uint32_t AlbedoTextureIndex = 0;
	};

	struct MeshSource
	{
		Buffer VertexData{};
		Buffer IndexBuffer{};
		uint32_t IndexCount;
	};

	struct MeshInstance
	{
		size_t SourceIndex;
		Math::Mat4 Transform;
	};

	struct Mesh
	{
		DynamicArray<MeshSource> Sources;
		DynamicArray<MeshInstance> Instances;
		DynamicArray<Image> Textures;
		DynamicArray<MeshMaterial> Materials;
		uint32_t MaterialOffset = 0;
	};

}
