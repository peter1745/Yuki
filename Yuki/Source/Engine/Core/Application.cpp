#include "Core/Application.hpp"
#include "EventSystem/ApplicationEvents.hpp"

namespace Yuki {

	Application::Application(const std::string& InName, RenderAPI InRenderAPI)
	    : m_Name(InName), m_RenderingAPI(InRenderAPI)
	{
	}

	GenericWindow* Application::NewWindow(WindowAttributes InWindowAttributes)
	{
		InWindowAttributes.EventCallback = [this](Event* InEvent) { m_EventSystem->PostEvent(InEvent); };
		Unique<GenericWindow> window = GenericWindow::New(m_RenderContext.GetPtr(), InWindowAttributes);
		window->Create();
		return m_Windows.emplace_back(std::move(window)).GetPtr();
	}

	void Application::Initialize()
	{
		m_RenderContext = RenderContext::New(m_RenderingAPI);
		m_RenderContext->Initialize();

		WindowAttributes windowAttributes =
		{
			.Title = m_Name,
			.Width = 1920,
			.Height = 1080,
			.Maximized = true,
		};

		m_MainWindow = NewWindow(windowAttributes);

		m_EventSystem = Unique<EventSystem>::Create();
		m_EventSystem->AddListener(this, &Application::OnWindowClose);

		m_MainWindow->Show();

		OnInitialize();

		m_RunEngineLoop = true;
	}

	void Application::Run()
	{
		while (m_RunEngineLoop)
		{
			for (const auto& window : m_Windows)
				window->ProcessEvents();

			OnRunLoop();

			// Clean up closed windows
			const auto it = std::ranges::remove_if(m_Windows, [this](const auto& InWindow) { return std::ranges::find(m_ClosedWindows, InWindow.GetPtr()) != m_ClosedWindows.end(); });
			m_Windows.erase(it.begin(), it.end());
		}
	}

	void Application::Destroy()
	{
		m_RenderContext->Destroy();
	}

	void Application::OnWindowClose(const WindowCloseEvent& InEvent)
	{
		m_ClosedWindows.emplace_back(InEvent.Window);

		if (InEvent.Window != m_MainWindow)
			return;

		ApplicationCloseEvent closeEvent;
		m_EventSystem->PostEvent(&closeEvent);
		m_RunEngineLoop = false;
	}

}
