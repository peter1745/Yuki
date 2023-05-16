#include "WindowsWindow.hpp"
#include "WindowsUtils.hpp"

#include <source_location>

namespace Yuki {

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WindowAttributes* windowAttributes = reinterpret_cast<WindowAttributes*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (uMsg)
		{
		case WM_NCCREATE:
		case WM_CREATE:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			break;
		}
		case WM_SIZE:
		{
			int32_t width = LOWORD(lParam);
			int32_t height = HIWORD(lParam);
			
			windowAttributes->Width = uint32_t(width);
			windowAttributes->Height = uint32_t(height);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
			break;
		}

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
		    &m_Attributes);

		YUKI_VERIFY(m_WindowHandle != nullptr, "Failed to create Win32 Window!");
	}

	void WindowsWindow::Show()
	{
		ShowWindow(m_WindowHandle, m_Attributes.Maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
	}

	void WindowsWindow::ProcessEvents()
	{
		MSG message = {};
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (message.message == WM_QUIT)
			m_Closed = true;
	}

	Unique<GenericWindow> GenericWindow::New(WindowAttributes InAttributes)
	{
		return Unique<WindowsWindow>::Create(std::move(InAttributes));
	}

}
