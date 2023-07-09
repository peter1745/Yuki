#pragma once

#include "Yuki/Core/Core.hpp"
#include "Yuki/Math/Mat4.hpp"
#include "Yuki/Math/Vec2.hpp"

namespace Yuki {

	struct Vertex
	{
		Math::Vec3 Position;
		Math::Vec3 Normal;
		Math::Vec2 UV;
		uint32_t MaterialIndex = 0; // TODO(Peter): Change to material handle?
	};

	struct MaterialData
	{
		int32_t AlbedoTextureIndex = -1;
		uint32_t AlbedoColor = 0;
	};

	struct MeshSource
	{
		DynamicArray<Vertex> Vertices;
		DynamicArray<uint32_t> Indices;
	};

	struct MeshNode
	{
		Math::Vec3 Translation;
		Math::Quat Rotation;
		Math::Vec3 Scale;
		int32_t MeshIndex;
	};

	struct MeshScene
	{
		DynamicArray<MaterialData> Materials;
		DynamicArray<MeshSource> Meshes;
		DynamicArray<MeshNode> Nodes;
	};

}
