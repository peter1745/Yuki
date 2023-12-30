#include "App.hpp"
#include "Window.hpp"

#include "Engine/Input/InputSystem.hpp"

namespace Yuki {

	Application::Application()
	{
		m_WindowSystem = Unique<WindowSystem>::New();
		m_InputSystem = Unique<InputSystem>::New();
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
	}

}
