#include "Rendering/MeshGenerator.hpp"

namespace Yuki {

	/*MeshGenerator::MeshGenerator(ResourceRegistry<MeshHandle, Mesh>& InMeshRegistry)
		: m_MeshRegistry(InMeshRegistry)
	{
	}

	MeshHandle MeshGenerator::GenerateCubeMesh(const Math::Vec3& InHalfSize)
	{*/
		/*DynamicArray<Vertex> vertices;
		DynamicArray<uint32_t> indices;

		float size = 1.0f;
		bool clockwise = false;
		bool flipLRVertical = false;

		constexpr Math::Vec3 normals[] { { 0, 0, 1 }, { 0, 0, -1 }, { -1, 0, 0 }, { 1, 0,  0 }, { 0, 1, 0 }, { 0, -1, 0 } };
		constexpr Math::Vec3 tangentsNormal[] { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, 1 }, { 0, 0, -1 }, { 1, 0, 0 }, { 1, 0, 0 } };
		constexpr Math::Vec3 tangentsFlipped[] { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { 1, 0, 0 }, { 1, 0, 0 } };

		const Math::Vec3* tangents = flipLRVertical ? tangentsFlipped : tangentsNormal;

		for (uint32_t side = 0; side < 6; side++)
		{
			auto N = normals[side];
			auto T = tangents[side];
			auto B = N.Cross(T);

			uint32_t offset = uint32_t(vertices.size());

			for (float dx = -1.0f; dx <= 1.0f; dx += 2.0f)
			{
				for (float dy = -1.0f; dy <= 1.0f; dy += 2.0f)
				{
					vertices.push_back(Vertex {
						.Position = 0.5f * size * (N + (T * dx) + (B * dy)),
						.Normal = N,
						.UV = {
							dx * 0.5f + 0.5f,
							dy * 0.5f + 0.5f,
						},
						.MaterialIndex = ((side / 2) + 2) % 3
					});
				}
			}

			if (clockwise)
			{
				indices.push_back(offset + 0);
				indices.push_back(offset + 3);
				indices.push_back(offset + 2);

				indices.push_back(offset + 0);
				indices.push_back(offset + 1);
				indices.push_back(offset + 3);
			}
			else
			{
				indices.push_back(offset + 0);
				indices.push_back(offset + 2);
				indices.push_back(offset + 3);

				indices.push_back(offset + 0);
				indices.push_back(offset + 3);
				indices.push_back(offset + 1);
			}
		}

		return m_RenderInterface->CreateMesh(vertices, indices);*/

		/*
			TODO(Peter):
				1. Store meshes in a ResourceRegistry (possibly owned by the user application or a WorldRoot component)
				2. Move mesh data upload out of MeshLoader into some other container (this function only needs the uploading, not the file loading / parsing)
				3. Consider utilizing the transfer system for the actual uploading (would be more feasable with the registry in place)
		*/

		/*cubeMeshID = scene.CreateMesh(
			vertices.size() * sizeof(Vertex), vertices.data(), u32(sizeof(Vertex)), 0,
			u32(indices.size()), indices.data())*/
	/*	return MeshHandle{};
	}*/

}
