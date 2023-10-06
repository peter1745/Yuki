#include "Engine/Common/WindowSystem.hpp"

#include "Engine/Common/StringHelper.hpp"

#include "Engine/Messages/EngineMessages.hpp"
#include "Engine/Messages/WindowMessages.hpp"

namespace Yuki {

	static const wchar_t* WindowClassName = L"YukiWindowsWindowClass";

	LRESULT CALLBACK WindowProc(HWND InHWND, UINT InMessage, WPARAM InWParam, LPARAM InLParam)
	{
		auto* Data = reinterpret_cast<WindowSystem::WindowData*>(GetWindowLongPtr(InHWND, GWLP_USERDATA));

		switch (InMessage)
		{
		case WM_NCCREATE:
		case WM_CREATE:
		{
			auto UserData = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(InLParam)->lpCreateParams);
			SetWindowLongPtr(InHWND, GWLP_USERDATA, UserData);
			break;
		}
		case WM_SIZE:
		{
			Data->Width = LOWORD(InLParam);
			Data->Height = HIWORD(InLParam);
			break;
		}
		case WM_CLOSE:
		{
			EngineMessages::Get().Post<WindowCloseMessage>({
				.Handle = Data->Handle,
				.IsPrimaryWindow = Data->IsPrimary
			});
			break;
		}
		default:
			break;
		}

		return DefWindowProc(InHWND, InMessage, InWParam, InLParam);
	}

	WindowSystem::WindowSystem()
	{
		WNDCLASSEXW WndClass = {};
		WndClass.cbSize = sizeof(WNDCLASSEXW);
		WndClass.lpszClassName = WindowClassName;
		WndClass.hInstance = GetModuleHandle(NULL);
		WndClass.lpfnWndProc = WindowProc;
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		RegisterClassExW(&WndClass);
	}

	WindowSystem::~WindowSystem()
	{
		UnregisterClassW(WindowClassName, GetModuleHandle(NULL));
	}

	WindowHandle WindowSystem::NewWindow(WindowInfo InInfo)
	{
		//YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		HMODULE Module = GetModuleHandle(NULL);

		auto Title = StringHelper::WideFromUTF8(InInfo.Title);
		WindowHandle Handle;

		auto& Window = m_Windows[Handle];
		Window.Handle = Handle;
		Window.IsPrimary = m_NextWindowIndex - 1 == 0;

		auto NativeHandle = CreateWindowExW(
			0,
			WindowClassName,
			Title.c_str(),

			WS_OVERLAPPEDWINDOW,

			(GetSystemMetrics(SM_CXSCREEN) / 2) - InInfo.Width / 2,
			(GetSystemMetrics(SM_CYSCREEN) / 2) - InInfo.Height / 2,
			InInfo.Width, InInfo.Height,

			nullptr,
			nullptr,
			Module,
			&Window);

		ShowWindow(NativeHandle, SW_SHOWNORMAL);

		RECT ClientRect;
		GetClientRect(NativeHandle, &ClientRect);
		Window.Width = ClientRect.right;
		Window.Height = ClientRect.bottom;

		Window.NativeHandle = NativeHandle;
		return Handle;
	}

	void WindowSystem::PollMessages()
	{
		MSG Message = {};

		for (const auto& Data : m_Windows | std::views::values)
		{
			while (PeekMessage(&Message, Cast<HWND>(Data.NativeHandle), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}

}