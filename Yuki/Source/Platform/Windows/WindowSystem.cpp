#include "Engine/Common/WindowSystem.hpp"

#include "Engine/Common/StringHelper.hpp"

#include "Engine/Messages/EngineMessages.hpp"
#include "Engine/Messages/WindowMessages.hpp"

namespace Yuki {

	static const wchar_t* WindowClassName = L"YukiWindowsWindowClass";
	static std::array<KeyCode, 512> s_KeyLookup;
	static std::array<RAWINPUT, 1000> s_RawInputBuffer;

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto* data = reinterpret_cast<WindowSystem::WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		switch (message)
		{
		case WM_NCCREATE:
		case WM_CREATE:
		{
			auto UserData = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, UserData);
			break;
		}
		case WM_SIZE:
		{
			data->Width = LOWORD(lParam);
			data->Height = HIWORD(lParam);
			break;
		}
		case WM_CLOSE:
		{
			EngineMessages::Get().Post<WindowCloseMessage>({
				.Handle = data->Handle,
				.IsPrimaryWindow = data->IsPrimary
			});
			break;
		}
		case WM_LBUTTONDOWN:
		{
			data->MouseButtonStates[MouseButton::Left] = true;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			data->MouseButtonStates[MouseButton::Middle] = true;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			data->MouseButtonStates[MouseButton::Right] = true;
			break;
		}
		case WM_LBUTTONUP:
		{
			data->MouseButtonStates[MouseButton::Left] = false;
			break;
		}
		case WM_MBUTTONUP:
		{
			data->MouseButtonStates[MouseButton::Middle] = false;
			break;
		}
		case WM_RBUTTONUP:
		{
			data->MouseButtonStates[MouseButton::Right] = false;
			break;
		}
		case WM_MOUSEMOVE:
		{
			data->MouseX = int32_t(LOWORD(lParam));
			data->MouseY = int32_t(HIWORD(lParam));
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			bool state = !(HIWORD(lParam) & KF_UP);

			int scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);

			KeyCode key = s_KeyLookup[scancode];

			if (wParam == VK_CONTROL)
			{
				if (HIWORD(lParam) & KF_EXTENDED)
				{
					key = KeyCode::RightControl;
				}
				else
				{
					// NOTE: Alt Gr sends Left Ctrl followed by Right Alt
					// HACK: We only want one event for Alt Gr, so if we detect
					//       this sequence we discard this Left Ctrl message now
					//       and later report Right Alt normally
					MSG next;
					const DWORD time = GetMessageTime();

					if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
					{
						if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN || next.message == WM_KEYUP || next.message == WM_SYSKEYUP)
						{
							if (next.wParam == VK_MENU && (HIWORD(next.lParam) & KF_EXTENDED) && next.time == time)
							{
								// Next message is Right Alt down so discard this
								break;
							}
						}
					}

					key = KeyCode::LeftControl;
				}
			}

			data->KeyStates[key] = state;

			/*if (!State)
				break;

			for (const auto* Context : data->InputContexts)
			{
				Context->ProcessInput(KeyInputEvent{ Key });
			}*/
			
			break;
		}
		default:
			break;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	WindowSystem::WindowSystem()
	{
		WNDCLASSEXW wndClass = {};
		wndClass.cbSize = sizeof(WNDCLASSEXW);
		wndClass.lpszClassName = WindowClassName;
		wndClass.hInstance = GetModuleHandle(NULL);
		wndClass.lpfnWndProc = WindowProc;
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		RegisterClassExW(&wndClass);
		
		{
			s_KeyLookup.fill(KeyCode::Unknown);

			s_KeyLookup[0x00B] = KeyCode::Num0;
			s_KeyLookup[0x002] = KeyCode::Num1;
			s_KeyLookup[0x003] = KeyCode::Num2;
			s_KeyLookup[0x004] = KeyCode::Num3;
			s_KeyLookup[0x005] = KeyCode::Num4;
			s_KeyLookup[0x006] = KeyCode::Num5;
			s_KeyLookup[0x007] = KeyCode::Num6;
			s_KeyLookup[0x008] = KeyCode::Num7;
			s_KeyLookup[0x009] = KeyCode::Num8;
			s_KeyLookup[0x00A] = KeyCode::Num9;
			s_KeyLookup[0x01E] = KeyCode::A;
			s_KeyLookup[0x030] = KeyCode::B;
			s_KeyLookup[0x02E] = KeyCode::C;
			s_KeyLookup[0x020] = KeyCode::D;
			s_KeyLookup[0x012] = KeyCode::E;
			s_KeyLookup[0x021] = KeyCode::F;
			s_KeyLookup[0x022] = KeyCode::G;
			s_KeyLookup[0x023] = KeyCode::H;
			s_KeyLookup[0x017] = KeyCode::I;
			s_KeyLookup[0x024] = KeyCode::J;
			s_KeyLookup[0x025] = KeyCode::K;
			s_KeyLookup[0x026] = KeyCode::L;
			s_KeyLookup[0x032] = KeyCode::M;
			s_KeyLookup[0x031] = KeyCode::N;
			s_KeyLookup[0x018] = KeyCode::O;
			s_KeyLookup[0x019] = KeyCode::P;
			s_KeyLookup[0x010] = KeyCode::Q;
			s_KeyLookup[0x013] = KeyCode::R;
			s_KeyLookup[0x01F] = KeyCode::S;
			s_KeyLookup[0x014] = KeyCode::T;
			s_KeyLookup[0x016] = KeyCode::U;
			s_KeyLookup[0x02F] = KeyCode::V;
			s_KeyLookup[0x011] = KeyCode::W;
			s_KeyLookup[0x02D] = KeyCode::X;
			s_KeyLookup[0x015] = KeyCode::Y;
			s_KeyLookup[0x02C] = KeyCode::Z;

			s_KeyLookup[0x028] = KeyCode::Apostrophe;
			//s_KeyLookup[0x02B] = GLFW_KEY_BACKSLASH;
			s_KeyLookup[0x033] = KeyCode::Comma;
			//s_KeyLookup[0x00D] = GLFW_KEY_EQUAL;
			//s_KeyLookup[0x029] = GLFW_KEY_GRAVE_ACCENT;
			//s_KeyLookup[0x01A] = GLFW_KEY_LEFT_BRACKET;
			s_KeyLookup[0x00C] = KeyCode::Minus;
			s_KeyLookup[0x034] = KeyCode::Period;
			//s_KeyLookup[0x01B] = GLFW_KEY_RIGHT_BRACKET;
			//s_KeyLookup[0x027] = GLFW_KEY_SEMICOLON;
			//s_KeyLookup[0x035] = GLFW_KEY_SLASH;
			//s_KeyLookup[0x056] = GLFW_KEY_WORLD_2;

			s_KeyLookup[0x00E] = KeyCode::Backspace;
			s_KeyLookup[0x153] = KeyCode::Delete;
			s_KeyLookup[0x14F] = KeyCode::End;
			s_KeyLookup[0x01C] = KeyCode::Enter;
			s_KeyLookup[0x001] = KeyCode::Escape;
			s_KeyLookup[0x147] = KeyCode::Home;
			s_KeyLookup[0x152] = KeyCode::Insert;
			//s_KeyLookup[0x15D] = GLFW_KEY_MENU;
			s_KeyLookup[0x151] = KeyCode::PageDown;
			s_KeyLookup[0x149] = KeyCode::PageUp;
			//s_KeyLookup[0x045] = GLFW_KEY_PAUSE;
			s_KeyLookup[0x039] = KeyCode::Space;
			s_KeyLookup[0x00F] = KeyCode::Tab;
			s_KeyLookup[0x03A] = KeyCode::CapsLock;
			//s_KeyLookup[0x145] = GLFW_KEY_NUM_LOCK;
			//s_KeyLookup[0x046] = GLFW_KEY_SCROLL_LOCK;
			s_KeyLookup[0x03B] = KeyCode::F1;
			s_KeyLookup[0x03C] = KeyCode::F2;
			s_KeyLookup[0x03D] = KeyCode::F3;
			s_KeyLookup[0x03E] = KeyCode::F4;
			s_KeyLookup[0x03F] = KeyCode::F5;
			s_KeyLookup[0x040] = KeyCode::F6;
			s_KeyLookup[0x041] = KeyCode::F7;
			s_KeyLookup[0x042] = KeyCode::F8;
			s_KeyLookup[0x043] = KeyCode::F9;
			s_KeyLookup[0x044] = KeyCode::F10;
			s_KeyLookup[0x057] = KeyCode::F11;
			s_KeyLookup[0x058] = KeyCode::F12;
			s_KeyLookup[0x064] = KeyCode::F13;
			s_KeyLookup[0x065] = KeyCode::F14;
			s_KeyLookup[0x066] = KeyCode::F15;
			s_KeyLookup[0x067] = KeyCode::F16;
			s_KeyLookup[0x068] = KeyCode::F17;
			s_KeyLookup[0x069] = KeyCode::F18;
			s_KeyLookup[0x06A] = KeyCode::F19;
			s_KeyLookup[0x06B] = KeyCode::F20;
			s_KeyLookup[0x06C] = KeyCode::F21;
			s_KeyLookup[0x06D] = KeyCode::F22;
			s_KeyLookup[0x06E] = KeyCode::F23;
			s_KeyLookup[0x076] = KeyCode::F24;
			s_KeyLookup[0x038] = KeyCode::LeftAlt;
			s_KeyLookup[0x01D] = KeyCode::LeftControl;
			s_KeyLookup[0x02A] = KeyCode::LeftShift;
			//s_KeyLookup[0x15B] = GLFW_KEY_LEFT_SUPER;
			//s_KeyLookup[0x137] = GLFW_KEY_PRINT_SCREEN;
			s_KeyLookup[0x138] = KeyCode::RightAlt;
			s_KeyLookup[0x11D] = KeyCode::RightControl;
			s_KeyLookup[0x036] = KeyCode::RightShift;
			//s_KeyLookup[0x15C] = GLFW_KEY_RIGHT_SUPER;
			s_KeyLookup[0x150] = KeyCode::DownArrow;
			s_KeyLookup[0x14B] = KeyCode::LeftArrow;
			s_KeyLookup[0x14D] = KeyCode::RightArrow;
			s_KeyLookup[0x148] = KeyCode::UpArrow;

			s_KeyLookup[0x052] = KeyCode::Numpad0;
			s_KeyLookup[0x04F] = KeyCode::Numpad1;
			s_KeyLookup[0x050] = KeyCode::Numpad2;
			s_KeyLookup[0x051] = KeyCode::Numpad3;
			s_KeyLookup[0x04B] = KeyCode::Numpad4;
			s_KeyLookup[0x04C] = KeyCode::Numpad5;
			s_KeyLookup[0x04D] = KeyCode::Numpad6;
			s_KeyLookup[0x047] = KeyCode::Numpad7;
			s_KeyLookup[0x048] = KeyCode::Numpad8;
			s_KeyLookup[0x049] = KeyCode::Numpad9;
			s_KeyLookup[0x04E] = KeyCode::NumpadAdd;
			s_KeyLookup[0x053] = KeyCode::NumpadDecimal;
			s_KeyLookup[0x135] = KeyCode::NumpadDivide;
			s_KeyLookup[0x11C] = KeyCode::NumpadEnter;
			s_KeyLookup[0x059] = KeyCode::NumpadEqual;
			s_KeyLookup[0x037] = KeyCode::NumpadMultiply;
			s_KeyLookup[0x04A] = KeyCode::NumpadSubtract;
		}
	}

	WindowSystem::~WindowSystem()
	{
		UnregisterClassW(WindowClassName, GetModuleHandle(NULL));
	}

	WindowHandle WindowSystem::NewWindow(WindowInfo info)
	{
		//YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		HMODULE module = GetModuleHandle(NULL);

		auto title = StringHelper::WideFromUTF8(info.Title);
		WindowHandle handle;

		auto& window = m_Windows[handle];
		window.Handle = handle;
		window.IsPrimary = m_NextWindowIndex - 1 == 0;

		auto nativeHandle = CreateWindowExW(
			0,
			WindowClassName,
			title.c_str(),

			WS_OVERLAPPEDWINDOW,

			(GetSystemMetrics(SM_CXSCREEN) / 2) - info.Width / 2,
			(GetSystemMetrics(SM_CYSCREEN) / 2) - info.Height / 2,
			info.Width, info.Height,

			nullptr,
			nullptr,
			module,
			&window);

		ShowWindow(nativeHandle, SW_SHOWNORMAL);

		RECT clientRect;
		GetClientRect(nativeHandle, &clientRect);
		window.Width = clientRect.right;
		window.Height = clientRect.bottom;

		const RAWINPUTDEVICE rawMouseInputDevice =
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = 0,
			.hwndTarget = nativeHandle
		};
		YUKI_VERIFY(RegisterRawInputDevices(&rawMouseInputDevice, 1, sizeof(RAWINPUTDEVICE)) != FALSE);

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		window.LastMouseDeltaX = cursorPos.x;
		window.LastMouseDeltaY = cursorPos.y;

		window.NativeHandle = nativeHandle;
		return handle;
	}

	void WindowSystem::PollMessages()
	{
		MSG message = {};

		UINT cbSize;

		if (GetRawInputBuffer(NULL, &cbSize, sizeof(RAWINPUTHEADER)) == UINT32_MAX)
			return;

		cbSize *= 1000;

#define YUKI_RAWINPUT_ALIGN(x)   (((x) + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1))
#define YUKI_NEXTRAWINPUTBLOCK(ptr) ((PRAWINPUT)YUKI_RAWINPUT_ALIGN((ULONG_PTR)((PBYTE)(ptr) + (ptr)->header.dwSize)))

		for (;;)
		{
			UINT cbSizeT = cbSize;
			UINT nInput = GetRawInputBuffer(s_RawInputBuffer.data(), &cbSizeT, sizeof(RAWINPUTHEADER));
			if (nInput == 0 || nInput == UINT32_MAX)
				break;

			PRAWINPUT current = s_RawInputBuffer.data();
			for (UINT i = 0; i < nInput; ++i)
			{
				for (auto& data : m_Windows | std::views::values)
				{
					data.LastMouseDeltaX += current->data.mouse.lLastX;
					data.LastMouseDeltaY += current->data.mouse.lLastY;
				}
				current = YUKI_NEXTRAWINPUTBLOCK(current);
			}
		}

		for (const auto& data : m_Windows | std::views::values)
		{
			if (data.Locked)
				SetCursorPos(data.LockedMouseX, data.LockedMouseY);

			while (PeekMessage(&message, Cast<HWND>(data.NativeHandle), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}

	void WindowSystem::SetCursorLock(WindowHandle handle, bool lock)
	{
		if (m_Windows[handle].Locked == lock)
			return;

		auto nativeHandle = Cast<HWND>(m_Windows[handle].NativeHandle);

		if (lock)
		{
			POINT lockPos;
			GetCursorPos(&lockPos);
			m_Windows[handle].LockedMouseX = lockPos.x;
			m_Windows[handle].LockedMouseY = lockPos.y;

			RECT clipRect;
			GetClientRect(nativeHandle, &clipRect);
			ClientToScreen(nativeHandle, reinterpret_cast<POINT*>(&clipRect.left));
			ClientToScreen(nativeHandle, reinterpret_cast<POINT*>(&clipRect.right));
			ClipCursor(&clipRect);
			ShowCursor(FALSE);
		}
		else
		{
			ClipCursor(NULL);
			ShowCursor(TRUE);
		}

		m_Windows[handle].Locked = lock;
	}

}