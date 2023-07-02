#pragma once

#include "Yuki/Math/Vec2.hpp"
#include "Yuki/Math/Mat4.hpp"

#include "RenderResources.hpp"

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
		int32_t AlbedoTextureIndex = -1;
		Math::Vec4 AlbedoColor{1.0f, 1.0f, 1.0f, 1.0f};
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
		DynamicArray<ImageHandle> Textures;
		DynamicArray<MeshMaterial> Materials;
		Buffer MaterialStorageBuffer{};
		uint32_t TextureOffset = 0;
	};

}
