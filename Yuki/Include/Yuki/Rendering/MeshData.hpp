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
		uint32_t MaterialIndex;
	};

	struct MeshMaterial
	{
		uint32_t AlbedoTextureIndex;
	};

	struct Mesh
	{
		List<Vertex> Vertices;
		List<uint32_t> Indices;

		Buffer* VertexBuffer;
		Buffer* IndexBuffer;
	};

	struct MeshInstance
	{
		Mesh* SourceMesh;
		Math::Mat4 Transform;
	};

	struct LoadedImage
	{
		uint32_t Width;
		uint32_t Height;
		List<std::byte> Data;
	};

	struct LoadedMesh
	{
		List<Mesh> Meshes;
		List<MeshInstance> Instances;
		List<LoadedImage> LoadedImages;
		List<Image2D*> Textures;
		List<MeshMaterial> Materials;
	};

}
