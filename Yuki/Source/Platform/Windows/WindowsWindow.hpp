#pragma once

#include "Core/GenericWindow.hpp"

namespace Yuki {

	class WindowsWindow : public GenericWindow
	{
	public:
		struct WindowData
		{
			WindowsWindow* This = nullptr;
			WindowAttributes* Attributes = nullptr;
			Map<KeyCode, KeyState> KeyStates;
			Map<MouseButton, MouseButtonState> MouseButtonStates;
		};

	public:
		WindowsWindow(RenderContext* InRenderContext, WindowAttributes InAttributes);
		~WindowsWindow();

		void Create() override;
		void Destroy() override;

		void Show() override;

		void ProcessEvents() override;

		SwapchainHandle GetSwapchain() const override { return m_Swapchain; }

		int64_t GetRawMouseDeltaX() override
		{
			int64_t delta = m_LastMouseDeltaX;
			m_LastMouseDeltaX = 0;
			return delta;
		}

		int64_t GetRawMouseDeltaY() override
		{
			int64_t delta = m_LastMouseDeltaY;
			m_LastMouseDeltaY = 0;
			return delta;
		}

		void SetCursorState(CursorState InState) override;

		bool IsKeyReleased(KeyCode InKeyCode) const override;
		bool IsKeyPressed(KeyCode InKeyCode) const override;

		bool IsMouseButtonReleased(MouseButton InButton) const override;
		bool IsMouseButtonPressed(MouseButton InButton) const override;

		HWND GetWindowHandle() const { return m_WindowHandle; }

	public:
		const WindowAttributes& GetAttributes() const override { return m_Attributes; }

	private:
		void ProcessRawInput();
		void RegisterKeyCodeTranslations();

	private:
		HWND m_WindowHandle = nullptr;
		WindowAttributes m_Attributes;
		RenderContext* m_RenderContext;
		SwapchainHandle m_Swapchain;

		int64_t m_LastMouseDeltaX = 0;
		int64_t m_LastMouseDeltaY = 0;

		CursorState m_CurrentCursorState = CursorState::Normal;

		POINT m_LockedCursorPosition;

	private:
		 WindowData m_WindowData;
	};

}
