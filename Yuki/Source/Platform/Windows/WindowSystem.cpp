#include "Engine/Common/WindowSystem.hpp"

#include "Engine/Common/StringHelper.hpp"

#include "Engine/Messages/EngineMessages.hpp"
#include "Engine/Messages/WindowMessages.hpp"

namespace Yuki {

	static const wchar_t* WindowClassName = L"YukiWindowsWindowClass";
	static std::array<KeyCode, 512> s_KeyLookup;
	static std::array<RAWINPUT, 1000> s_RawInputBuffer;

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
		case WM_LBUTTONDOWN:
		{
			Data->MouseButtonStates[MouseButton::Left] = true;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			Data->MouseButtonStates[MouseButton::Middle] = true;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			Data->MouseButtonStates[MouseButton::Right] = true;
			break;
		}
		case WM_LBUTTONUP:
		{
			Data->MouseButtonStates[MouseButton::Left] = false;
			break;
		}
		case WM_MBUTTONUP:
		{
			Data->MouseButtonStates[MouseButton::Middle] = false;
			break;
		}
		case WM_RBUTTONUP:
		{
			Data->MouseButtonStates[MouseButton::Right] = false;
			break;
		}
		case WM_MOUSEMOVE:
		{
			Data->MouseX = int32_t(LOWORD(InLParam));
			Data->MouseY = int32_t(HIWORD(InLParam));
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			bool State = !(HIWORD(InLParam) & KF_UP);

			int ScanCode = (HIWORD(InLParam) & (KF_EXTENDED | 0xff));
			if (ScanCode == 0)
				ScanCode = MapVirtualKeyW((UINT)InWParam, MAPVK_VK_TO_VSC);

			KeyCode Key = s_KeyLookup[ScanCode];

			if (InWParam == VK_CONTROL)
			{
				if (HIWORD(InLParam) & KF_EXTENDED)
				{
					Key = KeyCode::RightControl;
				}
				else
				{
					// NOTE: Alt Gr sends Left Ctrl followed by Right Alt
					// HACK: We only want one event for Alt Gr, so if we detect
					//       this sequence we discard this Left Ctrl message now
					//       and later report Right Alt normally
					MSG Next;
					const DWORD Time = GetMessageTime();

					if (PeekMessageW(&Next, NULL, 0, 0, PM_NOREMOVE))
					{
						if (Next.message == WM_KEYDOWN || Next.message == WM_SYSKEYDOWN || Next.message == WM_KEYUP || Next.message == WM_SYSKEYUP)
						{
							if (Next.wParam == VK_MENU && (HIWORD(Next.lParam) & KF_EXTENDED) && Next.time == Time)
							{
								// Next message is Right Alt down so discard this
								break;
							}
						}
					}

					Key = KeyCode::LeftControl;
				}
			}

			Data->KeyStates[Key] = State;

			/*if (!State)
				break;

			for (const auto* Context : Data->InputContexts)
			{
				Context->ProcessInput(KeyInputEvent{ Key });
			}*/
			
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

		const RAWINPUTDEVICE RawMouseInputDevice =
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = 0,
			.hwndTarget = NativeHandle
		};
		YUKI_VERIFY(RegisterRawInputDevices(&RawMouseInputDevice, 1, sizeof(RAWINPUTDEVICE)) != FALSE);

		POINT CursorPos;
		GetCursorPos(&CursorPos);
		Window.LastMouseDeltaX = CursorPos.x;
		Window.LastMouseDeltaY = CursorPos.y;

		Window.NativeHandle = NativeHandle;
		return Handle;
	}

	void WindowSystem::PollMessages()
	{
		MSG Message = {};

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
				for (auto& Data : m_Windows | std::views::values)
				{
					Data.LastMouseDeltaX += current->data.mouse.lLastX;
					Data.LastMouseDeltaY += current->data.mouse.lLastY;
				}
				current = YUKI_NEXTRAWINPUTBLOCK(current);
			}
		}

		for (const auto& Data : m_Windows | std::views::values)
		{
			if (Data.Locked)
				SetCursorPos(Data.LockedMouseX, Data.LockedMouseY);

			while (PeekMessage(&Message, Cast<HWND>(Data.NativeHandle), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}

	void WindowSystem::SetCursorLock(WindowHandle InHandle, bool InLock)
	{
		if (m_Windows[InHandle].Locked == InLock)
			return;

		HWND NativeHandle = Cast<HWND>(m_Windows[InHandle].NativeHandle);

		if (InLock)
		{
			POINT LockPos;
			GetCursorPos(&LockPos);
			m_Windows[InHandle].LockedMouseX = LockPos.x;
			m_Windows[InHandle].LockedMouseY = LockPos.y;

			RECT ClipRect;
			GetClientRect(NativeHandle, &ClipRect);
			ClientToScreen(NativeHandle, reinterpret_cast<POINT*>(&ClipRect.left));
			ClientToScreen(NativeHandle, reinterpret_cast<POINT*>(&ClipRect.right));
			ClipCursor(&ClipRect);
			ShowCursor(FALSE);
		}
		else
		{
			ClipCursor(NULL);
			ShowCursor(TRUE);
		}

		m_Windows[InHandle].Locked = InLock;
	}

}