#pragma once

#include "Unique.hpp"

#include <string_view>
#include <vector>

namespace Yuki {

	class Window
	{
	public:
		bool IsClosed() const { return m_Closed; }
		void Close() { m_Closed = true; }

	private:
		bool m_Closed = false;

		friend class WindowSystem;
	};

	class WindowSystem final
	{
	public:
		WindowSystem();
		~WindowSystem();

		Window* NewWindow(std::string_view title, uint32_t width = 1920, uint32_t height = 1080);

		void PollEvents() const;

	private:
		std::vector<Unique<Window>> m_Windows;
	};

}
