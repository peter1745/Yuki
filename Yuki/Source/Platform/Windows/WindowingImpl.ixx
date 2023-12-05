module;

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string_view>

export module Yuki.Windows:Windowing;

import Yuki.Core;
import Aura;

export {

	template<>
	struct Aura::HandleImpl<Yuki::Window>
	{
		HWND NativeHandle;
		bool IsClosed;
	};

}

namespace Yuki {

	struct WindowUserData
	{
		WindowSystem* System = nullptr;
		Aura::HandleImpl<Window>* Data = nullptr;
	};

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE || message == WM_CREATE)
		{
			SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
			return DefWindowProcA(hwnd, message, wParam, lParam);
		}

		auto* userData = reinterpret_cast<WindowUserData*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_CLOSE:
		{
			userData->Data->IsClosed = true;
			delete userData;
			break;
		}
		}

		return DefWindowProcA(hwnd, message, wParam, lParam);
	}

	bool Window::IsClosed() const
	{
		return m_Impl->IsClosed;
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

	Window WindowSystem::NewWindow(std::string_view title, uint32_t width, uint32_t height)
	{
		auto* impl = new Aura::HandleImpl<Window>();

		auto* userData = new WindowUserData();
		userData->System = this;
		userData->Data = impl;

		impl->NativeHandle = CreateWindowExA(
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
			userData
		);

		ShowWindow(impl->NativeHandle, SW_SHOWNORMAL);

		Window window = { impl };
		m_Windows.push_back(window);
		return window;
	}

	void WindowSystem::DestroyWindow(Window& window)
	{
		delete window.m_Impl;
		window.m_Impl = nullptr;
	}

	void WindowSystem::PollEvents() const
	{
		MSG msg;

		for (const auto window : m_Windows)
		{
			while (PeekMessageA(&msg, window->NativeHandle, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

}
