#include "WindowImpl.hpp"

namespace Yuki {

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE || message == WM_CREATE)
		{
			SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
			return DefWindowProcA(hwnd, message, wParam, lParam);
		}

		auto* window = reinterpret_cast<Window::Impl*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_CLOSE:
		{
			window->Closed = true;
			break;
		}
		}

		return DefWindowProcA(hwnd, message, wParam, lParam);
	}

	bool Window::IsClosed() const
	{
		return m_Impl->Closed;
	}

	WindowSystem::WindowSystem()
	{
		WNDCLASSEXA windowClass =
		{
			.cbSize = sizeof(WNDCLASSEXA),
			.lpfnWndProc = WindowProc,
			.hInstance = GetModuleHandle(nullptr),
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = "YukiWindowClass",
		};

		RegisterClassExA(&windowClass);
	}

	WindowSystem::~WindowSystem()
	{

	}

	Window WindowSystem::NewWindow(std::string_view title, uint32_t width, uint32_t height)
	{
		auto* window = new Window::Impl();

		window->WindowHandle = CreateWindowExA(
			NULL,
			"YukiWindowClass",
			title.data(),
			WS_OVERLAPPEDWINDOW,
			(GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2),
			(GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2),
			width,
			height,
			NULL,
			NULL,
			GetModuleHandle(nullptr),
			window
		);

		m_Windows.push_back({ window });

		ShowWindow(window->WindowHandle, SW_SHOWNORMAL);

		return { window };
	}

	void WindowSystem::PollEvents() const
	{
		MSG msg;

		for (auto window : m_Windows)
		{
			while (PeekMessageA(&msg, window->WindowHandle, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

}
