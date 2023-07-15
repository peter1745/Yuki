#pragma once

#include "Yuki/Core/Core.hpp"
#include "Yuki/Math/Mat4.hpp"
#include "Yuki/Math/Vec2.hpp"
#include "Yuki/Asset/AssetID.hpp"

namespace Yuki {

	struct Vertex
	{
		Math::Vec3 Position;
		Math::Vec3 Normal;
		Math::Vec3 UV;
		uint32_t MaterialIndex = 0; // TODO(Peter): Change to material handle?
	};

	struct MaterialData
	{
		int32_t AlbedoTextureIndex = -1;
		uint32_t AlbedoColor;
	};

	struct MeshSource
	{
		DynamicArray<Vertex> Vertices;
		DynamicArray<uint32_t> Indices;
	};

	struct MeshNode
	{
		std::string Name;
		Math::Vec3 Translation;
		Math::Quat Rotation;
		Math::Vec3 Scale;
		int32_t MeshIndex;

		DynamicArray<size_t> ChildNodes;
	};

	struct MeshScene
	{
		DynamicArray<MaterialData> Materials;
		DynamicArray<MeshSource> Meshes;
		size_t RootNodeIndex;
		DynamicArray<MeshNode> Nodes;
		DynamicArray<AssetID> Textures;
	};

}
