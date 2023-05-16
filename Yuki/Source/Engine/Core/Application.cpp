#include "Core/Application.hpp"
#include "EventSystem/ApplicationEvents.hpp"

namespace Yuki {

	Application::Application(const std::string& InName)
	    : m_Name(InName)
	{
	}

	void Application::Initialize()
	{
		WindowAttributes windowAttributes =
		{
			.Title = m_Name,
			.Width = 1920,
			.Height = 1080,
			.Maximized = true,
			.EventCallback = [this](Event* InEvent) { m_EventSystem->PostEvent(InEvent); }
		};

		m_Window = GenericWindow::New(windowAttributes);
		m_Window->Create();

		m_EventSystem = Unique<EventSystem>::Create();
		m_EventSystem->AddListener(this, &Application::OnWindowClose);

		m_RenderContext = RenderContext::New(m_Window.GetPtr());
		m_RenderContext->Initialize();

		OnInitialize();

		m_Window->Show();

		m_RunEngineLoop = true;
	}

	void Application::Run()
	{
		while (m_RunEngineLoop)
		{
			m_Window->ProcessEvents();
			OnRunLoop();
		}
	}

	void Application::Destroy()
	{
		m_RenderContext->Destroy();
	}

	void Application::OnWindowClose(const WindowCloseEvent& InEvent)
	{
		ApplicationCloseEvent closeEvent;
		m_EventSystem->PostEvent(&closeEvent);
		m_RunEngineLoop = false;
	}

}
