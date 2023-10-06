#pragma once

#include "Engine/Common/Types.hpp"
#include "Engine/Common/UniqueID.hpp"
#include "Engine/Input/InputContext.hpp"

namespace Yuki {

	struct WindowInfo
	{
		std::string Title;
		uint32_t Width = 1920;
		uint32_t Height = 1080;
	};

	using WindowHandle = UniqueID;

	class WindowSystem final
	{
	public:
		struct WindowData
		{
			WindowHandle Handle;
			uint32_t Width;
			uint32_t Height;
			void* NativeHandle;
			bool IsPrimary = false;

			DynamicArray<const InputContext*> InputContexts;
			HashMap<KeyCode, bool> KeyStates;
			HashMap<MouseButton, bool> MouseButtonStates;
			int32_t MouseX = 0;
			int32_t MouseY = 0;

			bool Locked = false;
			int32_t LockedMouseX = 0;
			int32_t LockedMouseY = 0;

			int64_t LastMouseDeltaX = 0;
			int64_t LastMouseDeltaY = 0;
		};

	public:
		WindowSystem();
		~WindowSystem();

		WindowHandle NewWindow(WindowInfo InInfo);
		const WindowData& GetWindowData(WindowHandle InHandle) const { return m_Windows.at(InHandle); }

		void PollMessages();

		void AddInputContext(WindowHandle InHandle, const InputContext* InContext)
		{
			m_Windows[InHandle].InputContexts.push_back(InContext);
		}

		bool GetKeyState(WindowHandle InHandle, KeyCode InKey) { return m_Windows.at(InHandle).KeyStates[InKey]; }
		bool GetMouseButtonState(WindowHandle InHandle, MouseButton InButton) { return m_Windows.at(InHandle).MouseButtonStates[InButton]; }
		int32_t GetMousePositionX(WindowHandle InHandle) { return m_Windows.at(InHandle).MouseX; }
		int32_t GetMousePositionY(WindowHandle InHandle) { return m_Windows.at(InHandle).MouseY; }

		int64_t GetRawMouseDeltaX(WindowHandle InHandle)
		{
			int64_t delta = m_Windows[InHandle].LastMouseDeltaX;
			m_Windows[InHandle].LastMouseDeltaX = 0;
			return delta;
		}

		int64_t GetRawMouseDeltaY(WindowHandle InHandle)
		{
			int64_t delta = m_Windows[InHandle].LastMouseDeltaY;
			m_Windows[InHandle].LastMouseDeltaY = 0;
			return delta;
		}

		void SetCursorLock(WindowHandle InHandle, bool InLock);

	private:
		HashMap<WindowHandle, WindowData> m_Windows;
		size_t m_NextWindowIndex = 0;
	};

}
