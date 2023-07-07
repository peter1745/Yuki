#pragma once

#include "EditorPanel.hpp"

#include <filesystem>

namespace YukiEditor {

	class ContentBrowser : public EditorPanel
	{
	public:
		ContentBrowser();

		void Draw() override;

	private:
		std::filesystem::path m_CurrentDirectory;
	};

}
