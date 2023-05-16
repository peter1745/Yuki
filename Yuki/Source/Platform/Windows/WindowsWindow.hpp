#pragma once

#include "Core/GenericWindow.hpp"

namespace Yuki {

	class WindowsWindow : public GenericWindow
	{
	public:
		WindowsWindow(WindowAttributes InAttributes);

		void Create() override;

		void Show() override;

		void ProcessEvents() const override;

		HWND GetWindowHandle() const { return m_WindowHandle; }

	public:
		const WindowAttributes& GetAttributes() const override { return m_Attributes; }

	private:
		HWND m_WindowHandle = nullptr;
		WindowAttributes m_Attributes;
	};

}
