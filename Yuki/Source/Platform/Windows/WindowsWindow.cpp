#include "WindowsWindow.hpp"
#include "WindowsUtils.hpp"

#include <source_location>

namespace Yuki {

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto* windowData = reinterpret_cast<WindowsWindow::WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

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
			
			WindowResizeEvent resizeEvent;
			resizeEvent.Window = windowData->This;
			resizeEvent.OldWidth = windowData->Attributes->Width;
			resizeEvent.OldHeight = windowData->Attributes->Height;
			resizeEvent.NewWidth = uint32_t(width);
			resizeEvent.NewHeight = uint32_t(height);

			windowData->Attributes->Width = resizeEvent.NewWidth;
			windowData->Attributes->Height = resizeEvent.NewHeight;

			if (windowData->Attributes->EventCallback)
				windowData->Attributes->EventCallback(&resizeEvent);

			break;
		}
		case WM_CLOSE:
		{
			WindowCloseEvent closeEvent;
			closeEvent.Window = windowData->This;
			if (windowData->Attributes->EventCallback)
				windowData->Attributes->EventCallback(&closeEvent);
			break;
		}
		default:
			break;
		}

		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	static bool s_WindowClassRegistered = false;

	WindowsWindow::WindowsWindow(RenderContext* InRenderContext, WindowAttributes InAttributes)
	    : m_Attributes(std::move(InAttributes)), m_RenderContext(InRenderContext)
	{
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::Create()
	{
		YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		const wchar_t* WindowClassName = L"YukiWindowsWindowClass";
		
		HMODULE hInstance = GetModuleHandle(nullptr);

		if (!s_WindowClassRegistered)
		{
			WNDCLASSEX windowClass = {};
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.lpszClassName = WindowClassName;
			windowClass.hInstance = hInstance;
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			RegisterClassEx(&windowClass);

			s_WindowClassRegistered = true;
		}

		auto title = WindowsUtils::ConvertUtf8ToWide(m_Attributes.Title);

		m_WindowData.This = this;
		m_WindowData.Attributes = &m_Attributes;

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
		    hInstance,
		    &m_WindowData);

		YUKI_VERIFY(m_WindowHandle != nullptr, "Failed to create Win32 Window!");

		m_Viewport = m_RenderContext->CreateViewport(this);
	}

	void WindowsWindow::Destroy()
	{
		m_RenderContext->DestroyViewport(m_Viewport);
		m_Viewport = nullptr;
	}

	void WindowsWindow::Show()
	{
		ShowWindow(m_WindowHandle, m_Attributes.Maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
	}

	void WindowsWindow::ProcessEvents() const
	{
		MSG message = {};
		while (PeekMessage(&message, m_WindowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	Unique<GenericWindow> GenericWindow::New(RenderContext* InRenderContext, WindowAttributes InAttributes)
	{
		return Unique<WindowsWindow>::Create(InRenderContext, std::move(InAttributes));
	}

}
