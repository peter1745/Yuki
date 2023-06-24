#include "Core/Application.hpp"
#include "Core/Timer.hpp"

#include "EventSystem/ApplicationEvents.hpp"

namespace Yuki {

	Application::Application(const std::string& InName, RenderAPI InRenderAPI)
		: m_Name(InName), m_RenderingAPI(InRenderAPI)
	{
	}

	GenericWindow* Application::NewWindow(WindowAttributes InWindowAttributes)
	{
		InWindowAttributes.EventCallback = [this](Event* InEvent) { m_EventSystem->PostEvent(InEvent); };
		Unique<GenericWindow> window = GenericWindow::New(m_RenderContext, InWindowAttributes);
		window->Create();
		return m_Windows.emplace_back(std::move(window)).GetPtr();
	}

	void Application::Initialize()
	{
		m_RenderContext = RenderContext::New(m_RenderingAPI);

		m_EventSystem = Unique<EventSystem>::Create();
		m_EventSystem->AddListener(this, &Application::OnWindowClose);

		OnInitialize();

		m_RunEngineLoop = true;
	}

	void Application::Run()
	{
		while (m_RunEngineLoop)
		{
			Timer timer("RunLoop");

			for (const auto& window : m_Windows)
				window->ProcessEvents();

			OnRunLoop(m_DeltaTime);

			// Clean up closed windows
			auto it = std::remove_if(m_Windows.begin(), m_Windows.end(), [this](const auto& InWindow) { return std::find(m_ClosedWindows.begin(), m_ClosedWindows.end(), InWindow.GetPtr()) != m_ClosedWindows.end(); });
			if (it != m_Windows.end())
				m_Windows.erase(it);

			m_DeltaTime = timer.ElapsedSeconds();
		}
	}

	void Application::Destroy()
	{
		OnDestroy();
		m_Windows.clear();
	}

	void Application::OnWindowClose(const WindowCloseEvent& InEvent)
	{
		m_ClosedWindows.emplace_back(InEvent.Window);

		InEvent.Window->Destroy();

		if (InEvent.Window != m_Windows[0].GetPtr())
			return;

		ApplicationCloseEvent closeEvent;
		m_EventSystem->PostEvent(&closeEvent);
		m_RunEngineLoop = false;
	}

}
