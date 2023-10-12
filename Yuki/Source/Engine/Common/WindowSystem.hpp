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

		WindowHandle NewWindow(WindowInfo info);
		const WindowData& GetWindowData(WindowHandle handle) const { return m_Windows.at(handle); }

		void PollMessages();

		void AddInputContext(WindowHandle handle, const InputContext* context)
		{
			m_Windows[handle].InputContexts.push_back(context);
		}

		bool GetKeyState(WindowHandle handle, KeyCode key) { return m_Windows.at(handle).KeyStates[key]; }
		bool GetMouseButtonState(WindowHandle handle, MouseButton button) { return m_Windows.at(handle).MouseButtonStates[button]; }
		int32_t GetMousePositionX(WindowHandle handle) { return m_Windows.at(handle).MouseX; }
		int32_t GetMousePositionY(WindowHandle handle) { return m_Windows.at(handle).MouseY; }

		int64_t GetRawMouseDeltaX(WindowHandle handle)
		{
			int64_t delta = m_Windows[handle].LastMouseDeltaX;
			m_Windows[handle].LastMouseDeltaX = 0;
			return delta;
		}

		int64_t GetRawMouseDeltaY(WindowHandle handle)
		{
			int64_t delta = m_Windows[handle].LastMouseDeltaY;
			m_Windows[handle].LastMouseDeltaY = 0;
			return delta;
		}

		void SetCursorLock(WindowHandle handle, bool lock);

	private:
		HashMap<WindowHandle, WindowData> m_Windows;
		size_t m_NextWindowIndex = 0;
	};

}
