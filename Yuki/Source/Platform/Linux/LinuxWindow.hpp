#pragma once

#include "Core/GenericWindow.hpp"

struct xcb_connection_t;
struct xcb_screen_t;
struct xcb_intern_atom_reply_t;

namespace Yuki {

	class LinuxWindow : public GenericWindow
	{
	public:
		LinuxWindow(RenderContext* InRenderContext, WindowAttributes InAttributes);
		~LinuxWindow();

		void Create() override;
		void Destroy() override;

		void Show() override;

		void ProcessEvents() override;

		Viewport* GetViewport() const override { return m_Viewport; }

		uint32_t GetWindowHandle() const { return m_WindowHandle; }

		xcb_connection_t* GetConnection() const;

	public:
		const WindowAttributes& GetAttributes() const override { return m_Attributes; }

	private:
		void HandleWindowInstanceEvent(void* InEvent, uint8_t InEventType);

	private:
		xcb_connection_t* m_Connection = nullptr;
		xcb_screen_t* m_Screen = nullptr;
		uint32_t m_WindowHandle = 0;
		WindowAttributes m_Attributes;

		xcb_intern_atom_reply_t* m_ProtocolsReply = nullptr;
		xcb_intern_atom_reply_t* m_DeleteWindowReply = nullptr;

		RenderContext* m_RenderContext = nullptr;
		Viewport* m_Viewport = nullptr;
	};

}
