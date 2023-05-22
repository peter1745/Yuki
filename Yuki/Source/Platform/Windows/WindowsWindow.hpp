#pragma once

#include "Core/GenericWindow.hpp"

namespace Yuki {

	class WindowsWindow : public GenericWindow
	{
	public:
		struct WindowData
		{
			WindowsWindow* This = nullptr;
			WindowAttributes* Attributes = nullptr;
		};

	public:
		WindowsWindow(RenderContext* InRenderContext, WindowAttributes InAttributes);
		~WindowsWindow();

		void Create() override;

		void Show() override;

		void ProcessEvents() const override;

		Swapchain& GetSwapchain() const override { return *m_Swapchain; }

		HWND GetWindowHandle() const { return m_WindowHandle; }

	public:
		const WindowAttributes& GetAttributes() const override { return m_Attributes; }

	private:
		HWND m_WindowHandle = nullptr;
		WindowAttributes m_Attributes;
		RenderContext* m_RenderContext;
		Swapchain* m_Swapchain = nullptr;

	private:
		 WindowData m_WindowData;
	};

}
