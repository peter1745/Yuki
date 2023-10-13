#pragma once

#include "Engine/Common/Core.hpp"

namespace Yuki {

	struct ShadingAttributes
	{
		Vec3 Normal;
		Vec2 TexCoord;
	};

	struct MeshData
	{
		DynamicArray<Vec3> Positions;
		DynamicArray<ShadingAttributes> ShadingAttributes;
		DynamicArray<uint32_t> Indices;
	};

	struct MeshNode
	{
		std::string Name;
		Vec3 Translation;
		Quat Rotation;
		Vec3 Scale;

		int32_t MeshIndex = -1;

		DynamicArray<size_t> ChildNodes;
	};

	struct MeshScene
	{
		DynamicArray<size_t> NodeIndices;
	};

	struct Model
	{
		DynamicArray<MeshData> Meshes;
		DynamicArray<MeshNode> Nodes;
	};

	class glTFLoader
	{
	public:
		void Load(const std::filesystem::path& filepath, Model& model);
	};

}
