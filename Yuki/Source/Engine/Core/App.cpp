#include "App.hpp"
#include "Window.hpp"

#include "Engine/Input/InputSystemImpl.hpp"

namespace Yuki {

	Application::Application()
	{
		Detail::InitializeLogging();

		m_WindowSystem = Unique<WindowSystem>::New();

		m_InputSystem = { new InputSystem::Impl() };
		m_InputSystem->Init();
	}

	void Application::Run()
	{
		OnRun();

		while (m_Running)
		{
			m_WindowSystem->PollEvents();
			m_InputSystem->Update();

			OnUpdate();
		}

		m_InputSystem->Shutdown();
	}

}
