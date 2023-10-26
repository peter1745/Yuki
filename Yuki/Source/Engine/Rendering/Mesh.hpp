#pragma once

namespace Yuki {

	struct ShadingAttributes
	{
		Vec3 Normal;
		Vec2 TexCoord;
	};

	struct Mesh
	{
		DynamicArray<Vec3> Positions;
		DynamicArray<ShadingAttributes> ShadingAttributes;
		DynamicArray<uint32_t> Indices;
		uint32_t MaterialIndex;
	};

	struct MeshInstance
	{
		std::string Name;
		Mat4 Transform;
		uint32_t MeshIndex = 0;
	};

	struct MeshScene
	{
		DynamicArray<MeshInstance> Instances;
	};

	struct MeshMaterial
	{
		uint32_t BaseColor = 0xffffffff;
		int32_t BaseColorTextureIndex = -1;
		uint32_t AlphaBlending = 0;
	};

	struct MeshTexture
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		DynamicArray<std::byte> Data;
	};

	struct Model
	{
		DynamicArray<Mesh> Meshes;
		DynamicArray<MeshMaterial> Materials;
		DynamicArray<MeshTexture> Textures;
		DynamicArray<MeshScene> Scenes;
	};
}
