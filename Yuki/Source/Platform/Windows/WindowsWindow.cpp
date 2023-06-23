#include "WindowsWindow.hpp"
#include "WindowsUtils.hpp"

#include <source_location>

namespace Yuki {

	static bool s_WindowClassRegistered = false;
	static std::array<RAWINPUT, 1000> s_RawInputBuffer;
	static std::array<KeyCode, 512> s_KeyCodes;

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
		case WM_LBUTTONDOWN:
		{
			windowData->MouseButtonStates[MouseButton::Left] = MouseButtonState::Pressed;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			windowData->MouseButtonStates[MouseButton::Middle] = MouseButtonState::Pressed;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			windowData->MouseButtonStates[MouseButton::Right] = MouseButtonState::Pressed;
			break;
		}
		case WM_LBUTTONUP:
		{
			windowData->MouseButtonStates[MouseButton::Left] = MouseButtonState::Released;
			break;
		}
		case WM_MBUTTONUP:
		{
			windowData->MouseButtonStates[MouseButton::Middle] = MouseButtonState::Released;
			break;
		}
		case WM_RBUTTONUP:
		{
			windowData->MouseButtonStates[MouseButton::Right] = MouseButtonState::Released;
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			KeyState state = (HIWORD(lParam) & KF_UP) ? KeyState::Released : KeyState::Pressed;

			int scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);

			KeyCode key = s_KeyCodes[scancode];

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

			windowData->KeyStates[key] = state;
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
			WNDCLASSEXW windowClass = {};
			windowClass.cbSize = sizeof(WNDCLASSEXW);
			windowClass.lpszClassName = WindowClassName;
			windowClass.hInstance = hInstance;
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			RegisterClassExW(&windowClass);

			s_WindowClassRegistered = true;
		}

		auto title = WindowsUtils::ConvertUtf8ToWide(m_Attributes.Title);

		m_WindowData.This = this;
		m_WindowData.Attributes = &m_Attributes;

		m_WindowHandle = CreateWindowExW(
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

		const RAWINPUTDEVICE rawMouseInputDevice =
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = 0,
			.hwndTarget = m_WindowHandle
		};
		YUKI_VERIFY(RegisterRawInputDevices(&rawMouseInputDevice, 1, sizeof(RAWINPUTDEVICE)) != FALSE);

		POINT cursorPos = {};
		GetCursorPos(&cursorPos);
		m_LastMouseDeltaX = cursorPos.x;
		m_LastMouseDeltaY = cursorPos.y;

		m_Swapchain = m_RenderContext->CreateSwapchain(this);

		RegisterKeyCodeTranslations();
	}

	void WindowsWindow::Destroy()
	{
		m_RenderContext->Destroy(m_Swapchain);
	}

	void WindowsWindow::Show()
	{
		ShowWindow(m_WindowHandle, m_Attributes.Maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
	}

	void WindowsWindow::ProcessRawInput()
	{
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
				m_LastMouseDeltaX += current->data.mouse.lLastX;
				m_LastMouseDeltaY += current->data.mouse.lLastY;
				current = YUKI_NEXTRAWINPUTBLOCK(current);
			}
		}
	}

	void WindowsWindow::RegisterKeyCodeTranslations()
	{
		s_KeyCodes.fill(KeyCode::Unknown);

		s_KeyCodes[0x00B] = KeyCode::Num0;
		s_KeyCodes[0x002] = KeyCode::Num1;
		s_KeyCodes[0x003] = KeyCode::Num2;
		s_KeyCodes[0x004] = KeyCode::Num3;
		s_KeyCodes[0x005] = KeyCode::Num4;
		s_KeyCodes[0x006] = KeyCode::Num5;
		s_KeyCodes[0x007] = KeyCode::Num6;
		s_KeyCodes[0x008] = KeyCode::Num7;
		s_KeyCodes[0x009] = KeyCode::Num8;
		s_KeyCodes[0x00A] = KeyCode::Num9;
		s_KeyCodes[0x01E] = KeyCode::A;
		s_KeyCodes[0x030] = KeyCode::B;
		s_KeyCodes[0x02E] = KeyCode::C;
		s_KeyCodes[0x020] = KeyCode::D;
		s_KeyCodes[0x012] = KeyCode::E;
		s_KeyCodes[0x021] = KeyCode::F;
		s_KeyCodes[0x022] = KeyCode::G;
		s_KeyCodes[0x023] = KeyCode::H;
		s_KeyCodes[0x017] = KeyCode::I;
		s_KeyCodes[0x024] = KeyCode::J;
		s_KeyCodes[0x025] = KeyCode::K;
		s_KeyCodes[0x026] = KeyCode::L;
		s_KeyCodes[0x032] = KeyCode::M;
		s_KeyCodes[0x031] = KeyCode::N;
		s_KeyCodes[0x018] = KeyCode::O;
		s_KeyCodes[0x019] = KeyCode::P;
		s_KeyCodes[0x010] = KeyCode::Q;
		s_KeyCodes[0x013] = KeyCode::R;
		s_KeyCodes[0x01F] = KeyCode::S;
		s_KeyCodes[0x014] = KeyCode::T;
		s_KeyCodes[0x016] = KeyCode::U;
		s_KeyCodes[0x02F] = KeyCode::V;
		s_KeyCodes[0x011] = KeyCode::W;
		s_KeyCodes[0x02D] = KeyCode::X;
		s_KeyCodes[0x015] = KeyCode::Y;
		s_KeyCodes[0x02C] = KeyCode::Z;

		s_KeyCodes[0x028] = KeyCode::Apostrophe;
		//s_KeyCodes[0x02B] = GLFW_KEY_BACKSLASH;
		s_KeyCodes[0x033] = KeyCode::Comma;
		//s_KeyCodes[0x00D] = GLFW_KEY_EQUAL;
		//s_KeyCodes[0x029] = GLFW_KEY_GRAVE_ACCENT;
		//s_KeyCodes[0x01A] = GLFW_KEY_LEFT_BRACKET;
		s_KeyCodes[0x00C] = KeyCode::Minus;
		s_KeyCodes[0x034] = KeyCode::Period;
		//s_KeyCodes[0x01B] = GLFW_KEY_RIGHT_BRACKET;
		//s_KeyCodes[0x027] = GLFW_KEY_SEMICOLON;
		//s_KeyCodes[0x035] = GLFW_KEY_SLASH;
		//s_KeyCodes[0x056] = GLFW_KEY_WORLD_2;

		s_KeyCodes[0x00E] = KeyCode::Backspace;
		s_KeyCodes[0x153] = KeyCode::Delete;
		s_KeyCodes[0x14F] = KeyCode::End;
		s_KeyCodes[0x01C] = KeyCode::Enter;
		s_KeyCodes[0x001] = KeyCode::Escape;
		s_KeyCodes[0x147] = KeyCode::Home;
		s_KeyCodes[0x152] = KeyCode::Insert;
		//s_KeyCodes[0x15D] = GLFW_KEY_MENU;
		s_KeyCodes[0x151] = KeyCode::PageDown;
		s_KeyCodes[0x149] = KeyCode::PageUp;
		//s_KeyCodes[0x045] = GLFW_KEY_PAUSE;
		s_KeyCodes[0x039] = KeyCode::Space;
		s_KeyCodes[0x00F] = KeyCode::Tab;
		s_KeyCodes[0x03A] = KeyCode::CapsLock;
		//s_KeyCodes[0x145] = GLFW_KEY_NUM_LOCK;
		//s_KeyCodes[0x046] = GLFW_KEY_SCROLL_LOCK;
		s_KeyCodes[0x03B] = KeyCode::F1;
		s_KeyCodes[0x03C] = KeyCode::F2;
		s_KeyCodes[0x03D] = KeyCode::F3;
		s_KeyCodes[0x03E] = KeyCode::F4;
		s_KeyCodes[0x03F] = KeyCode::F5;
		s_KeyCodes[0x040] = KeyCode::F6;
		s_KeyCodes[0x041] = KeyCode::F7;
		s_KeyCodes[0x042] = KeyCode::F8;
		s_KeyCodes[0x043] = KeyCode::F9;
		s_KeyCodes[0x044] = KeyCode::F10;
		s_KeyCodes[0x057] = KeyCode::F11;
		s_KeyCodes[0x058] = KeyCode::F12;
		s_KeyCodes[0x064] = KeyCode::F13;
		s_KeyCodes[0x065] = KeyCode::F14;
		s_KeyCodes[0x066] = KeyCode::F15;
		s_KeyCodes[0x067] = KeyCode::F16;
		s_KeyCodes[0x068] = KeyCode::F17;
		s_KeyCodes[0x069] = KeyCode::F18;
		s_KeyCodes[0x06A] = KeyCode::F19;
		s_KeyCodes[0x06B] = KeyCode::F20;
		s_KeyCodes[0x06C] = KeyCode::F21;
		s_KeyCodes[0x06D] = KeyCode::F22;
		s_KeyCodes[0x06E] = KeyCode::F23;
		s_KeyCodes[0x076] = KeyCode::F24;
		s_KeyCodes[0x038] = KeyCode::LeftAlt;
		s_KeyCodes[0x01D] = KeyCode::LeftControl;
		s_KeyCodes[0x02A] = KeyCode::LeftShift;
		//s_KeyCodes[0x15B] = GLFW_KEY_LEFT_SUPER;
		//s_KeyCodes[0x137] = GLFW_KEY_PRINT_SCREEN;
		s_KeyCodes[0x138] = KeyCode::RightAlt;
		s_KeyCodes[0x11D] = KeyCode::RightControl;
		s_KeyCodes[0x036] = KeyCode::RightShift;
		//s_KeyCodes[0x15C] = GLFW_KEY_RIGHT_SUPER;
		s_KeyCodes[0x150] = KeyCode::DownArrow;
		s_KeyCodes[0x14B] = KeyCode::LeftArrow;
		s_KeyCodes[0x14D] = KeyCode::RightArrow;
		s_KeyCodes[0x148] = KeyCode::UpArrow;

		s_KeyCodes[0x052] = KeyCode::Numpad0;
		s_KeyCodes[0x04F] = KeyCode::Numpad1;
		s_KeyCodes[0x050] = KeyCode::Numpad2;
		s_KeyCodes[0x051] = KeyCode::Numpad3;
		s_KeyCodes[0x04B] = KeyCode::Numpad4;
		s_KeyCodes[0x04C] = KeyCode::Numpad5;
		s_KeyCodes[0x04D] = KeyCode::Numpad6;
		s_KeyCodes[0x047] = KeyCode::Numpad7;
		s_KeyCodes[0x048] = KeyCode::Numpad8;
		s_KeyCodes[0x049] = KeyCode::Numpad9;
		s_KeyCodes[0x04E] = KeyCode::NumpadAdd;
		s_KeyCodes[0x053] = KeyCode::NumpadDecimal;
		s_KeyCodes[0x135] = KeyCode::NumpadDivide;
		s_KeyCodes[0x11C] = KeyCode::NumpadEnter;
		s_KeyCodes[0x059] = KeyCode::NumpadEqual;
		s_KeyCodes[0x037] = KeyCode::NumpadMultiply;
		s_KeyCodes[0x04A] = KeyCode::NumpadSubtract;
	}

	void WindowsWindow::ProcessEvents()
	{
		ProcessRawInput();

		if (m_CurrentCursorState == CursorState::Locked)
			SetCursorPos(m_LockedCursorPosition.x, m_LockedCursorPosition.y);

		MSG message = {};
		while (PeekMessage(&message, m_WindowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	void WindowsWindow::SetCursorState(CursorState InState)
	{
		if (InState == m_CurrentCursorState)
			return;

		switch (InState)
		{
		case CursorState::Normal:
		{
			ClipCursor(NULL);
			ShowCursor(TRUE);
			break;
		}
		case CursorState::Locked:
		{
			GetCursorPos(&m_LockedCursorPosition);

			RECT clipRect;
			GetClientRect(m_WindowHandle, &clipRect);
			ClientToScreen(m_WindowHandle, (POINT*)&clipRect.left);
			ClientToScreen(m_WindowHandle, (POINT*)&clipRect.right);
			ClipCursor(&clipRect);
			ShowCursor(FALSE);
			break;
		}
		}

		m_CurrentCursorState = InState;
	}

	bool WindowsWindow::IsKeyReleased(KeyCode InKeyCode) const
	{
		return m_WindowData.KeyStates.contains(InKeyCode) && m_WindowData.KeyStates.at(InKeyCode) == KeyState::Released;
	}

	bool WindowsWindow::IsKeyPressed(KeyCode InKeyCode) const
	{
		return m_WindowData.KeyStates.contains(InKeyCode) && m_WindowData.KeyStates.at(InKeyCode) == KeyState::Pressed;
	}

	bool WindowsWindow::IsMouseButtonReleased(MouseButton InButton) const
	{
		return m_WindowData.MouseButtonStates.contains(InButton) && m_WindowData.MouseButtonStates.at(InButton) == MouseButtonState::Released;
	}

	bool WindowsWindow::IsMouseButtonPressed(MouseButton InButton) const
	{
		return m_WindowData.MouseButtonStates.contains(InButton) && m_WindowData.MouseButtonStates.at(InButton) == MouseButtonState::Pressed;
	}

	Unique<GenericWindow> GenericWindow::New(RenderContext* InRenderContext, WindowAttributes InAttributes)
	{
		return Unique<WindowsWindow>::Create(InRenderContext, std::move(InAttributes));
	}

}
