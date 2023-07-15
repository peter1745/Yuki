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
						.UV = Math::Vec3{
							(dx * 0.5f + 0.5f) * InUVMultiplier,
							(dy * 0.5f + 0.5f) * InUVMultiplier,
							0.0f
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
			material.AlbedoTextureIndex = 0;
		}

		LogInfo("Generated {} vertices, {} triangles", mesh.Vertices.size(), mesh.Indices.size() / 3);

		auto* meshAsset = new MeshAsset();
		meshAsset->Scene = meshScene;
		return InAssetSystem.AddAsset(AssetType::Mesh, meshAsset);
	}

	AssetID MeshGenerator::GenerateIcosphere(AssetSystem& InAssetSystem, uint32_t InSubdivisions, float InUVMultiplier)
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

		constexpr Vertex BaseVertices[] =
		{
			{ { 0.0f, 1.0f, 0.0f } },
			{ { -1.0f, 0.0f, 0.0f } },
			{ { 0.0f, 0.0f, 1.0f } },
			{ { 1.0f, 0.0f, 0.0f } },
			{ { 0.0f, 0.0f, -1.0f } },
			{ { 0.0f, -1.0f, 0.0f } }
		};

		constexpr uint32_t VertexIndices[] =
		{
			0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1, 5, 1, 5, 2, 5, 3, 5, 4
		};

		constexpr uint32_t EdgeTriplets[] =
		{
			0, 1, 4, 1, 2, 5, 2, 3, 6, 3, 0, 7, 8, 9, 4, 9, 10, 5, 10, 11, 6, 11, 8, 7
		};

		const uint32_t verticesPerFace = ((InSubdivisions + 3) * (InSubdivisions + 3) - (InSubdivisions + 3)) / 2;
		const uint32_t vertexCount = verticesPerFace * 8 - (InSubdivisions + 2) * 12 + 6;
		const uint32_t trianglesPerFace = (InSubdivisions + 1) * (InSubdivisions + 1);

		mesh.Vertices.reserve(vertexCount);
		mesh.Indices.reserve(trianglesPerFace * 8 * 3);

		for (const auto& v : BaseVertices)
			mesh.Vertices.push_back(v);

		struct Edge
		{
			DynamicArray<uint32_t> Indices;
		};

		std::array<Edge, 12> edges;
		for (uint32_t i = 0; i < sizeof(VertexIndices) / sizeof(uint32_t); i += 2)
		{
			Math::Vec3 startVertex = mesh.Vertices[VertexIndices[i]].Position;
			Math::Vec3 endVertex = mesh.Vertices[VertexIndices[i + 1]].Position;

			auto& edge = edges[i / 2];
			edge.Indices.resize(InSubdivisions + 2, 0);
			edge.Indices[0] = VertexIndices[i];

			for (uint32_t j = 0; j < InSubdivisions; j++)
			{
				float t = (j + 1.0f) / (InSubdivisions + 1.0f);
				edge.Indices[j + 1] = uint32_t(mesh.Vertices.size());
				mesh.Vertices.push_back({
					.Position = Math::Vec3::Slerp(startVertex, endVertex, t)
				});
			}

			edge.Indices[InSubdivisions + 1] = VertexIndices[i + 1];
		}

		auto createFace = [&](Edge InEdgeA, Edge InEdgeB, Edge InBottom, bool InReverse)
		{
			uint32_t pointsInEdge = InEdgeA.Indices.size();
			DynamicArray<uint32_t> vertexMap;
			vertexMap.push_back(InEdgeA.Indices[0]);

			for (uint32_t i = 1; i < pointsInEdge - 1; i++)
			{
				vertexMap.push_back(InEdgeA.Indices[i]);

				auto vertexA = mesh.Vertices[InEdgeA.Indices[i]].Position;
				auto vertexB = mesh.Vertices[InEdgeB.Indices[i]].Position;

				uint32_t innerPoints = i - 1;
				for (uint32_t j = 0; j < innerPoints; j++)
				{
					float t = (j + 1.0) / (innerPoints + 1.0f);
					vertexMap.push_back(mesh.Vertices.size());
					mesh.Vertices.push_back({ Math::Vec3::Slerp(vertexA, vertexB, t) });
				}

				vertexMap.push_back(InEdgeB.Indices[i]);
			}

			for (uint32_t i = 0; i < pointsInEdge; i++)
				vertexMap.push_back(InBottom.Indices[i]);

			uint32_t rows = InSubdivisions + 1;
			for (uint32_t row = 0; row < rows; row++)
			{
				uint32_t topVertex = ((row + 1) * (row + 1) - row - 1) / 2;
				uint32_t bottomVertex = ((row + 2) * (row + 2) - row - 2) / 2;
				uint32_t trianglesInRow = 1 + 2 * row;

				for (uint32_t column = 0; column < trianglesInRow; column++)
				{
					uint32_t v0, v1, v2;

					if (column % 2 == 0)
					{
						v0 = topVertex;
						v1 = bottomVertex + 1;
						v2 = bottomVertex;
						topVertex++;
						bottomVertex++;
					}
					else
					{
						v0 = topVertex;
						v1 = bottomVertex;
						v2 = topVertex - 1;
					}

					mesh.Indices.push_back(vertexMap[v2]);
					mesh.Indices.push_back(InReverse ? vertexMap[v0] : vertexMap[v1]);
					mesh.Indices.push_back(InReverse ? vertexMap[v1] : vertexMap[v0]);
				}
			}
		};

		for (uint32_t i = 0; i < sizeof(EdgeTriplets) / sizeof(uint32_t); i += 3)
			createFace(edges[EdgeTriplets[i]], edges[EdgeTriplets[i + 1]], edges[EdgeTriplets[i + 2]], (i / 3) >= 4);

		for (auto& vertex : mesh.Vertices)
		{
			vertex.Normal = vertex.Position.Normalized();
			vertex.UV = vertex.Position * InUVMultiplier;
		}

		for (uint32_t i = 0; i < 3; i++)
		{
			auto& material = meshScene.Materials.emplace_back();
			material.AlbedoColor = 0xFFFFFFFF;
			material.AlbedoTextureIndex = 0;
		}

		LogInfo("Generated {} vertices, {} triangles", mesh.Vertices.size(), mesh.Indices.size() / 3);

		auto* meshAsset = new MeshAsset();
		meshAsset->Scene = meshScene;
		return InAssetSystem.AddAsset(AssetType::Mesh, meshAsset);
	}

}
