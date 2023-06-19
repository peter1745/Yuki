#include "IO/MeshLoader.hpp"

#include <fastgltf/parser.hpp>
#include <fastgltf/types.hpp>

namespace Yuki {

	void MeshLoader::LoadGLTFMesh(const std::filesystem::path& InFilePath)
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer dataBuffer;
		dataBuffer.loadFromFile(InFilePath);

		std::unique_ptr<fastgltf::glTF> gltfAsset;

		switch (fastgltf::determineGltfFileType(&dataBuffer))
		{
		case fastgltf::GltfType::glTF:
		{
			gltfAsset = parser.loadGLTF(&dataBuffer, InFilePath.parent_path());
			break;
		}
		case fastgltf::GltfType::GLB:
		{
			gltfAsset = parser.loadBinaryGLTF(&dataBuffer, InFilePath.parent_path());
			break;
		}
		}

		YUKI_VERIFY(parser.getError() == fastgltf::Error::None);
		YUKI_VERIFY(gltfAsset->parse() == fastgltf::Error::None);

		auto asset = gltfAsset->getParsedAsset();

		for (auto& mesh : asset->meshes)
		{
			for (auto& primitive : mesh.primitives)
			{
				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
					continue;

				if (!primitive.indicesAccessor.has_value())
					break;

				
			}
		}

		YUKI_VERIFY(false);
	}

}
