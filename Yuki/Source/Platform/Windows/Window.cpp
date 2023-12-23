#include "Engine/Core/Window.hpp"

#include "WindowsCommon.hpp"

namespace Yuki {

	static std::unordered_map<Window*, HWND> s_WindowHandles;

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE || message == WM_CREATE)
		{
			SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
			return DefWindowProcA(hwnd, message, wParam, lParam);
		}

		auto* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_CLOSE:
		{
			window->Close();
			break;
		}
		}

		return DefWindowProcA(hwnd, message, wParam, lParam);
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

	Window* WindowSystem::NewWindow(std::string_view title, uint32_t width, uint32_t height)
	{
		Unique<Window> window = Unique<Window>::New();
		Window* windowPtr = window.Raw();

		HWND handle = CreateWindowExA(
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
			windowPtr
		);

		s_WindowHandles[windowPtr] = handle;
		m_Windows.push_back(std::move(window));

		ShowWindow(handle, SW_SHOWNORMAL);

		return windowPtr;
	}

	void WindowSystem::PollEvents() const
	{
		MSG msg;

		for (auto[window, handle] : s_WindowHandles)
		{
			while (PeekMessageA(&msg, handle, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

}
