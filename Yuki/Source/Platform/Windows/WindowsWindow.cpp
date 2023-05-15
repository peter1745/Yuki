#include "WindowsWindow.hpp"
#include "WindowsUtils.hpp"

#include <source_location>

namespace Yuki {

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	WindowsWindow::WindowsWindow(WindowAttributes InAttributes)
	    : m_Attributes(std::move(InAttributes))
	{
	}

	void WindowsWindow::Create()
	{
		YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		const wchar_t* WindowClassName = L"YukiWindowsWindowClass";

		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.lpszClassName = WindowClassName;
		windowClass.hInstance = GetModuleHandle(nullptr);
		windowClass.lpfnWndProc = WindowProc;
		RegisterClassEx(&windowClass);

		auto title = WindowsUtils::ConvertUtf8ToWide(m_Attributes.Title);

		m_WindowHandle = CreateWindowEx(
		    0,
		    WindowClassName,
		    title.c_str(),

		    WS_OVERLAPPEDWINDOW,

		    (GetSystemMetrics(SM_CXSCREEN) / 2) - m_Attributes.Width / 2,
		    (GetSystemMetrics(SM_CYSCREEN) / 2) - m_Attributes.Height / 2,
			m_Attributes.Width, m_Attributes.Height,

		    nullptr,
		    nullptr,
		    windowClass.hInstance,
		    nullptr);

		YUKI_VERIFY(m_WindowHandle != nullptr, "Failed to create Win32 Window!");

		ShowWindow(m_WindowHandle, SW_SHOW);
	}

	void WindowsWindow::ProcessEvents() const
	{
		MSG message = {};
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	std::unique_ptr<GenericWindow> GenericWindow::New(WindowAttributes InAttributes)
	{
		return std::make_unique<WindowsWindow>(std::move(InAttributes));
	}

}
