#include "Rendering/MeshGenerator.hpp"
#include "Math/Math.hpp"

namespace Yuki {

	AssetID MeshGenerator::GenerateCubeSphere(AssetSystem& InAssetSystem, float InRadius, uint32_t InSegments, float InUVMultiplier)
	{
		MeshScene meshScene;
		auto& rootNode = meshScene.Nodes.emplace_back();
		rootNode.Name = "CubeSphere";
		rootNode.Translation = { 0.0f, 0.0f, 0.0f };
		rootNode.Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		rootNode.Scale = { 1.0f, 1.0f, 1.0f };
		rootNode.MeshIndex = 0;
		meshScene.RootNodeIndex = 0;

		auto& mesh = meshScene.Meshes.emplace_back();

		// Front Back | Left Right | Top Bottom
		constexpr Math::Vec3 normals[]{ { 0, 0, 1 }, { 0, 0, -1 }, { -1, 0, 0 }, { 1, 0,  0 }, { 0, 1, 0 }, { 0, -1, 0 } };

		// Sides V -> +Y, Top and Bottom V -> -Z
		constexpr Math::Vec3 tangentsNormal[]{ { 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, 1 }, { 0, 0, -1 }, { 1, 0, 0 }, { 1, 0, 0 } };

		// Front back V -> +Y, Left Right V -> -Y, Top Bottom V -> -Z
		constexpr Math::Vec3 tangentsFlipped[]{ { 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { 1, 0, 0 }, { 1, 0, 0 } };

		constexpr float PI = Math::PI<float>();
		Math::Quat rotations[]
		{
			Math::Quat(0.f, Math::Vec3{0.f, 1.f, 0.f}),
			Math::Quat(PI, Math::Vec3{0.f, 1.f, 0.f}),

			Math::Quat(-0.5f * PI, Math::Vec3{0.f, 1.f, 0.f}),
			Math::Quat(0.5f * PI, Math::Vec3{0.f, 1.f, 0.f}),

			Math::Quat(-0.5f * PI, Math::Vec3{1.f, 0.f, 0.f}),
			Math::Quat(0.5f * PI, Math::Vec3{1.f, 0.f, 0.f}),
		};

		//const Math::Vec3* tangents = flipLRVertical ? tangentsFlipped : tangentsNormal;
		const Math::Vec3* tangents = tangentsNormal;

		for (uint32_t side = 0; side < 6; side++)
		{
			auto N = normals[side];
			auto T = tangents[side];
			auto B = N.Cross(T);

			auto vertexOffset = static_cast<uint32_t>(mesh.Vertices.size());

			for (uint32_t latIndex = 0; latIndex <= InSegments; latIndex++)
			{
				for (uint32_t lonIndex = 0; lonIndex <= InSegments; lonIndex++)
				{
					float dy = 2.f * float(latIndex) / InSegments - 1.f;
					float dx = 2.f * float(lonIndex) / InSegments - 1.f;

					auto n = (N + (T * dx) + (B * dy)).Normalized();
					auto t = n.Cross(B);
					auto b = n.Cross(t);
					auto pos = n * InRadius;
					mesh.Vertices.push_back(Vertex{
						.Position = pos,
						.Normal = n,
						.UV = Math::Vec2{
							(dx * 0.5f + 0.5f) * InUVMultiplier,
							(dy * 0.5f + 0.5f) * InUVMultiplier,
						},
						.MaterialIndex = ((side / 2) + 2) % 3
					});
				}
			}

			for (uint32_t latIndex = 0; latIndex < InSegments; latIndex++)
			{
				uint32_t row1 = (InSegments + 1) * latIndex;
				uint32_t row2 = (InSegments + 1) * (latIndex + 1);

				for (uint32_t lonIndex = 0; lonIndex < InSegments; lonIndex++)
				{
					uint32_t l1 = lonIndex;
					uint32_t l2 = lonIndex + 1;

					l1 = l2;
					l2 = lonIndex;

					mesh.Indices.push_back(vertexOffset + row1 + l2);
					mesh.Indices.push_back(vertexOffset + row1 + l1);
					mesh.Indices.push_back(vertexOffset + row2 + l1);

					mesh.Indices.push_back(vertexOffset + row1 + l2);
					mesh.Indices.push_back(vertexOffset + row2 + l1);
					mesh.Indices.push_back(vertexOffset + row2 + l2);
				}
			}
		}

		for (uint32_t i = 0; i < 3; i++)
		{
			auto& material = meshScene.Materials.emplace_back();
			material.AlbedoColor = 0xFFFFFFFF;
			material.AlbedoTextureIndex = -1;
		}

		LogInfo("Generated {} vertices, {} triangles", mesh.Vertices.size(), mesh.Indices.size() / 3);

		auto* meshAsset = new MeshAsset();
		meshAsset->Scene = meshScene;
		return InAssetSystem.AddAsset(AssetType::Mesh, meshAsset);
	}

}
