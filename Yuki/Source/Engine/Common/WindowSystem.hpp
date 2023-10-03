#pragma once

#include "Engine/Common/Types.hpp"
#include "Engine/Common/UniqueID.hpp"

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
		};

	public:
		WindowSystem();
		~WindowSystem();

		WindowHandle NewWindow(WindowInfo InInfo);
		const WindowData& GetWindowData(WindowHandle InHandle) const { return m_Windows.at(InHandle); }

		void PollMessages();

	private:
		HashMap<WindowHandle, WindowData> m_Windows;
		size_t m_NextWindowIndex = 0;
	};

}
