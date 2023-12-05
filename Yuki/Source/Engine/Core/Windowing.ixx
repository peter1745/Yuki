module;

#include <string_view>
#include <vector>

export module Yuki.Core:Windowing;

import Aura;

export namespace Yuki {

	class WindowSystem;

	struct Window : Aura::Handle<Window>
	{
		bool IsClosed() const;

		friend WindowSystem;
	};

	class WindowSystem
	{
	public:
		WindowSystem();

		Window NewWindow(std::string_view title, uint32_t width, uint32_t height);
		void DestroyWindow(Window& window);

		void PollEvents() const;

	private:
		std::vector<Window> m_Windows;
	};

}
