#pragma once

#include "EditorPanel.hpp"

#include <Yuki/Asset/AssetRegistry.hpp>

#include <filesystem>

namespace YukiEditor {

	class ContentBrowser : public EditorPanel
	{
	public:
		ContentBrowser(Yuki::AssetRegistry& InAssetRegistry);

		void Draw() override;

	private:
		Yuki::AssetRegistry& m_AssetRegistry;
		std::filesystem::path m_CurrentDirectory;

		Yuki::DynamicArray<Yuki::AssetID> m_AssetItems;
	};

}
