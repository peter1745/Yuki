#pragma once

#include "Core/GenericWindow.hpp"

namespace Yuki {

	class WindowsWindow : public GenericWindow
	{
	public:
		WindowsWindow(WindowAttributes InAttributes);

		void Create() override;

		void Show() override;

		void ProcessEvents() override;

		bool ShouldClose() const override { return m_Closed; }

	public:
		const WindowAttributes& GetAttributes() const override { return m_Attributes; }

	private:
		HWND m_WindowHandle = nullptr;
		WindowAttributes m_Attributes;
		bool m_Closed = false;
	};

}
