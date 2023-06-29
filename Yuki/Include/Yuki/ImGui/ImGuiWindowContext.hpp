#pragma once

namespace Yuki {

	class GenericWindow;

	class ImGuiWindowContext
	{
	public:
		ImGuiWindowContext(GenericWindow* InWindow);
		~ImGuiWindowContext();

		void NewFrame() const;
	};

}