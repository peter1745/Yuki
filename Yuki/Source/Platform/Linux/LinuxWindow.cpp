#include "LinuxWindow.hpp"
//#include "WindowsUtils.hpp"

#include <xcb/xcb.h>

#include <source_location>

namespace Yuki {

	static bool s_XCBInitialized = false;
	static xcb_connection_t* s_Connection;
	static xcb_screen_t* s_Screen;

	static xcb_intern_atom_reply_t* s_ProtocolsReply;
	static xcb_intern_atom_reply_t* s_DeleteWindowReply;

	LinuxWindow::LinuxWindow(RenderContext* InRenderContext, WindowAttributes InAttributes)
	    : m_Attributes(std::move(InAttributes)), m_RenderContext(InRenderContext)
	{
	}

	LinuxWindow::~LinuxWindow()
	{
	}

	static void InitXCB()
	{
		int screenp = 0;
		s_Connection = xcb_connect(NULL, &screenp);
		YUKI_VERIFY(!xcb_connection_has_error(s_Connection));

		xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(s_Connection));

		for (int i = screenp; i > 0; i--)
			xcb_screen_next(&iter);

		s_Screen = iter.data;
	}

	void LinuxWindow::Create()
	{
		YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		if (!s_XCBInitialized)
		{
			InitXCB();
			s_XCBInitialized = true;
		}

		m_WindowHandle = xcb_generate_id(s_Connection);

		uint32_t eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t events = XCB_EVENT_MASK_EXPOSURE;
		events |= XCB_EVENT_MASK_STRUCTURE_NOTIFY;
		events |= XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;
		events |= XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;

		uint32_t valueList[] = { s_Screen->white_pixel, events };

		xcb_create_window(s_Connection,
						XCB_COPY_FROM_PARENT,
						m_WindowHandle,
						s_Screen->root,
						0, 0,
					   	m_Attributes.Width, m_Attributes.Height,
						0,
						XCB_WINDOW_CLASS_INPUT_OUTPUT,
						s_Screen->root_visual,
						eventMask,
						valueList);

		// Update Title
		xcb_change_property(s_Connection,
							XCB_PROP_MODE_REPLACE,
							m_WindowHandle,
							XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
							8,
							m_Attributes.Title.length(), m_Attributes.Title.c_str());

		// Hook up close button
		xcb_intern_atom_cookie_t windowDeleteCookie = xcb_intern_atom(s_Connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
		xcb_intern_atom_cookie_t protocolsCookie = xcb_intern_atom(s_Connection, 1, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
		s_DeleteWindowReply = xcb_intern_atom_reply(s_Connection, windowDeleteCookie, nullptr);
		s_ProtocolsReply = xcb_intern_atom_reply(s_Connection, protocolsCookie, nullptr);
		xcb_change_property(s_Connection,
							XCB_PROP_MODE_REPLACE,
							m_WindowHandle,
							s_ProtocolsReply->atom, 4, 32, 1, &s_DeleteWindowReply->atom);

		xcb_map_window(s_Connection, m_WindowHandle);
		xcb_flush(s_Connection);

		m_Viewport = m_RenderContext->CreateViewport(this);
	}

	void LinuxWindow::Destroy()
	{
		m_RenderContext->DestroyViewport(m_Viewport);
		m_Viewport = nullptr;
	}

	void LinuxWindow::Show()
	{
	}

	void LinuxWindow::ProcessEvents()
	{
		xcb_generic_event_t* event;

		while ((event = xcb_poll_for_event(s_Connection)))
		{
			switch (event->response_type & ~0x80)
			{
			case XCB_CONFIGURE_NOTIFY:
			{
				xcb_configure_notify_event_t* configureEvent = (xcb_configure_notify_event_t*)event;

				WindowResizeEvent resizeEvent;
				resizeEvent.Window = this;
				resizeEvent.OldWidth = m_Attributes.Width;
				resizeEvent.OldHeight = m_Attributes.Height;
				resizeEvent.NewWidth = configureEvent->width;
				resizeEvent.NewWidth = configureEvent->height;

				m_Attributes.Width = configureEvent->width;
				m_Attributes.Height = configureEvent->height;

				if (m_Attributes.EventCallback)
					m_Attributes.EventCallback(&resizeEvent);

				break;
			}
			case XCB_CLIENT_MESSAGE:
			{
				xcb_client_message_event_t* clientMessageEvent = (xcb_client_message_event_t*)event;

				if (clientMessageEvent->data.data32[0] == s_DeleteWindowReply->atom)
				{
					WindowCloseEvent closeEvent;
					closeEvent.Window = this;

					if (m_Attributes.EventCallback)
						m_Attributes.EventCallback(&closeEvent);
				}

				break;
			}
			}

			free(event);
		}
	}

	xcb_connection_t* LinuxWindow::GetConnection() const { return s_Connection; }

	Unique<GenericWindow> GenericWindow::New(RenderContext* InRenderContext, WindowAttributes InAttributes)
	{
		return Unique<LinuxWindow>::Create(InRenderContext, std::move(InAttributes));
	}

}
