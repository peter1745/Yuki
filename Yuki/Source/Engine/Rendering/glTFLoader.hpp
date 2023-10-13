#pragma once

#include "Engine/Common/Core.hpp"

namespace Yuki {

	struct ShadingAttributes
	{
		Vec3 Normal;
		Vec2 TexCoord;
		uint32_t MaterialIndex;
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

	struct MeshMaterial
	{
		uint32_t BaseColor = 0xffffffff;
	};

	struct Model
	{
		DynamicArray<MeshData> Meshes;
		DynamicArray<MeshMaterial> Materials;
		DynamicArray<MeshNode> Nodes;
		DynamicArray<MeshScene> Scenes;
	};

	class glTFLoader
	{
	public:
		void Load(const std::filesystem::path& filepath, Model& model);
	};

}
