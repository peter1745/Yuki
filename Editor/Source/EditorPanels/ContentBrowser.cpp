#include "ContentBrowser.hpp"

#include <Yuki/Core/ScopeExitGuard.hpp>
#include <Yuki/Asset/AssetConverters.hpp>

#include <imgui/imgui.h>
#include <nfd.hpp>

namespace YukiEditor {

	ContentBrowser::ContentBrowser()
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
					NFD::UniquePathSet filePaths;
					nfdresult_t result = NFD::OpenDialogMultiple(filePaths, static_cast<const nfdnfilteritem_t*>(nullptr));

					if (result == NFD_OKAY)
					{
						nfdpathsetsize_t pathCount;
						NFD::PathSet::Count(filePaths, pathCount);

						for (uint32_t i = 0; i < pathCount; i++)
						{
							NFD::UniquePathSetPath path;
							NFD::PathSet::GetPath(filePaths, i, path);
							Yuki::MeshConverter converter;
							converter.Convert(path.get());
						}
					}
				}
			}
		}

		for (auto dir : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			ImGui::Button(dir.path().stem().string().c_str());
		}
	}

}
