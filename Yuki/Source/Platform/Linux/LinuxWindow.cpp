#include "LinuxWindow.hpp"

#include <xcb/xcb.h>

#include <source_location>

namespace Yuki {

	LinuxWindow::LinuxWindow(RenderContext* InRenderContext, WindowAttributes InAttributes)
	    : m_Attributes(std::move(InAttributes)), m_RenderContext(InRenderContext)
	{
	}

	LinuxWindow::~LinuxWindow()
	{
	}

	void LinuxWindow::Create()
	{
		YUKI_VERIFY(!m_WindowHandle, "Cannot create window multiple times!");

		int screenp = 0;
		m_Connection = xcb_connect(NULL, &screenp);
		YUKI_VERIFY(!xcb_connection_has_error(m_Connection));

		xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(m_Connection));

		for (int i = screenp; i > 0; i--)
			xcb_screen_next(&iter);

		m_Screen = iter.data;

		m_WindowHandle = xcb_generate_id(m_Connection);

		uint32_t eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t events = XCB_EVENT_MASK_EXPOSURE;
		events |= XCB_EVENT_MASK_STRUCTURE_NOTIFY;
		events |= XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;
		events |= XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;

		uint32_t valueList[] = { m_Screen->white_pixel, events };

		xcb_create_window(m_Connection,
						XCB_COPY_FROM_PARENT,
						m_WindowHandle,
						m_Screen->root,
						0, 0,
					   	m_Attributes.Width, m_Attributes.Height,
						0,
						XCB_WINDOW_CLASS_INPUT_OUTPUT,
						m_Screen->root_visual,
						eventMask,
						valueList);

		// Update Title
		xcb_change_property(m_Connection,
							XCB_PROP_MODE_REPLACE,
							m_WindowHandle,
							XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
							8,
							m_Attributes.Title.length(), m_Attributes.Title.c_str());

		// Hook up close button
		xcb_intern_atom_cookie_t windowDeleteCookie = xcb_intern_atom(m_Connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
		xcb_intern_atom_cookie_t protocolsCookie = xcb_intern_atom(m_Connection, 1, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
		m_DeleteWindowReply = xcb_intern_atom_reply(m_Connection, windowDeleteCookie, nullptr);
		m_ProtocolsReply = xcb_intern_atom_reply(m_Connection, protocolsCookie, nullptr);
		xcb_change_property(m_Connection,
							XCB_PROP_MODE_REPLACE,
							m_WindowHandle,
							m_ProtocolsReply->atom, 4, 32, 1, &m_DeleteWindowReply->atom);

		xcb_map_window(m_Connection, m_WindowHandle);
		xcb_flush(m_Connection);

		m_Viewport = m_RenderContext->CreateViewport(this);
	}

	void LinuxWindow::Destroy()
	{
		m_RenderContext->DestroyViewport(m_Viewport);
		m_Viewport = nullptr;

		xcb_destroy_window(m_Connection, m_WindowHandle);
		xcb_disconnect(m_Connection);
	}

	void LinuxWindow::Show()
	{
	}

	void LinuxWindow::ProcessEvents()
	{
		xcb_generic_event_t* event;

		while ((event = xcb_poll_for_event(m_Connection)))
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

				if (clientMessageEvent->data.data32[0] == m_DeleteWindowReply->atom)
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

	xcb_connection_t* LinuxWindow::GetConnection() const { return m_Connection; }

	Unique<GenericWindow> GenericWindow::New(RenderContext* InRenderContext, WindowAttributes InAttributes)
	{
		return Unique<LinuxWindow>::Create(InRenderContext, std::move(InAttributes));
	}

}
