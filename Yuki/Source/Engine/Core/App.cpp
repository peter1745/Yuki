#include "App.hpp"
#include "Window.hpp"

#include "Engine/Input/InputSystemImpl.hpp"

#include <ranges>

namespace Yuki {

	Application::Application()
	{
		Detail::InitializeLogging();

		m_WindowSystem = Aura::Unique<WindowSystem>::New();

		m_InputSystem = { new InputSystem::Impl() };
		m_InputSystem->Init();
	}

	void Application::Run()
	{
		OnRun();

		while (m_Running)
		{
			using namespace std::chrono;

			auto now = Clock::now();
			auto delta = duration_cast<milliseconds>(now - m_LastTime).count();
			auto deltaMicro = duration_cast<microseconds>(now - m_LastTime).count();
			m_LastTime = now;

			WriteLine("Delta: {}ms ({} micro)", delta, deltaMicro);

			m_WindowSystem->PollEvents();
			m_InputSystem->Update();

			OnUpdate();
		}

		OnShutdown();

		m_InputSystem->Shutdown();
	}

}
