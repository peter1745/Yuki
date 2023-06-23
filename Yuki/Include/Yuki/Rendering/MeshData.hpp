#pragma once

#include "Yuki/Math/Vec2.hpp"
#include "Yuki/Math/Mat4.hpp"

//#include "Yuki/Rendering/RHI/RenderContext.hpp"

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

	/*struct Mesh
	{
		List<Vertex> Vertices;
		List<uint32_t> Indices;

		Unique<Buffer> VertexBuffer = nullptr;
		Unique<Buffer> IndexBuffer = nullptr;
	};

	struct MeshInstance
	{
		Mesh* SourceMesh = nullptr;
		Math::Mat4 Transform;
	};

	struct LoadedImage
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		List<std::byte> Data;
	};

	struct LoadedMesh
	{
		List<Mesh> Meshes;
		List<MeshInstance> Instances;
		List<LoadedImage> LoadedImages;
		List<Unique<Image2D>> Textures;
		List<MeshMaterial> Materials;
	};*/

}
