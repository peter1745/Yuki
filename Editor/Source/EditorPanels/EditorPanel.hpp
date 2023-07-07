#pragma once

namespace YukiEditor {

	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void Draw() = 0;
	};

}
