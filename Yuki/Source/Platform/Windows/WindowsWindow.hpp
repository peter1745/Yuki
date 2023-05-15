#pragma once

#include "Core/GenericWindow.hpp"

namespace Yuki {

	class WindowsWindow : public GenericWindow
	{
	public:
		WindowsWindow(WindowAttributes InAttributes);

		void Create() override;
		void ProcessEvents() const override;

	private:
		HWND m_WindowHandle = nullptr;
		WindowAttributes m_Attributes;
	};

}
