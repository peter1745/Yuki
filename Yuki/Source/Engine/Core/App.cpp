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

		m_LastTime = Clock::now();

		while (m_Running)
		{
			using namespace std::chrono;

			auto now = Clock::now();
			m_AccumulatedFrames++;
			m_AccumulatedTime += duration_cast<Duration>(now - m_LastTime);
			m_LastTime = now;

			if (m_AccumulatedTime.count() >= 1000.0f)
			{
				WriteLine("Avg Frame Time: {:.3f}ms", m_AccumulatedTime.count() / m_AccumulatedFrames);
				m_AccumulatedFrames = 0;
				m_AccumulatedTime = Duration::zero();
			}

			m_WindowSystem->PollEvents();
			m_InputSystem->Update();

			OnUpdate();
		}

		OnShutdown();

		m_InputSystem->Shutdown();
	}

}
