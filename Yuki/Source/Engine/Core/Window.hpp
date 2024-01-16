#pragma once

#include "Unique.hpp"
#include "Handle.hpp"

#include <string_view>
#include <vector>

namespace Yuki {

	struct Window : Handle<Window>
	{
		bool IsClosed() const;
	};

	class WindowSystem final
	{
	public:
		WindowSystem();
		~WindowSystem();

		Window NewWindow(std::string_view title, uint32_t width = 1920, uint32_t height = 1080);

		void PollEvents() const;

	private:
		std::vector<Window> m_Windows;
	};

}
