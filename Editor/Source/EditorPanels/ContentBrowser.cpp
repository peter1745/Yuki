#include "ContentBrowser.hpp"

#include <Yuki/Core/Core.hpp>
#include <Yuki/Core/ScopeExitGuard.hpp>
#include <Yuki/Asset/AssetImporter.hpp>

#include <imgui/imgui.h>
#include <nfd.hpp>
#include <spdlog/fmt/fmt.h>

namespace YukiEditor {

	ContentBrowser::ContentBrowser(Yuki::AssetRegistry& InAssetRegistry)
		: m_AssetRegistry(InAssetRegistry)
	{
		m_CurrentDirectory = std::filesystem::current_path() / "Content";
	}

	void ContentBrowser::Draw()
	{
		YUKI_SCOPE_EXIT_GUARD() { ImGui::End(); };

		if (!ImGui::Begin("Content Browser"))
			return;

		if (ImGui::BeginPopupContextWindow())
		{
			YUKI_SCOPE_EXIT_GUARD() { ImGui::EndPopup(); };

			if (ImGui::BeginMenu("Import"))
			{
				YUKI_SCOPE_EXIT_GUARD() { ImGui::EndMenu(); };

				if (ImGui::MenuItem("Mesh"))
				{
					static const Yuki::Array<nfdfilteritem_t, 1> MeshFilters{ { "glTF", "gltf,glb" } };

					NFD::UniquePathSet filePaths;
					nfdresult_t result = NFD::OpenDialogMultiple(filePaths, MeshFilters.data(), uint32_t(MeshFilters.size()));

					if (result == NFD_OKAY)
					{
						nfdpathsetsize_t pathCount;
						NFD::PathSet::Count(filePaths, pathCount);

						for (uint32_t i = 0; i < pathCount; i++)
						{
							NFD::UniquePathSetPath path;
							NFD::PathSet::GetPath(filePaths, i, path);

							auto assetID = Yuki::AssetImporter<Yuki::MeshAsset>().Import(m_AssetRegistry, path.get());
							m_AssetItems.push_back(assetID);
						}

						m_AssetRegistry.Serialize();
					}
				}
			}
		}

		m_AssetRegistry.ForEach([&](auto InHandle, const auto& InMetadata)
		{
			auto label = InMetadata.FilePath.stem().string();
			if (ImGui::Button(label.c_str()))
				Yuki::AssetImporter<Yuki::MeshAsset>().Load(m_AssetRegistry, InHandle);
		});
	}

}
