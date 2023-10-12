#include "Application.hpp"

#include "EngineTime.hpp"
#include "Engine/Messages/EngineMessages.hpp"

namespace Yuki {

	void Application::Run()
	{
		EngineTime::GetInternal().m_LastTime = EngineTime::Clock::now();

		EngineMessages::Get().AddListener<WindowCloseMessage>([this](const auto& message)
		{
			ShouldClose = message.IsPrimaryWindow;
		});

		while (!ShouldClose)
		{
			EngineTime::CalculateDeltaTime();

			Update();

			EngineMessages::Get().ProcessMessages();
		}
	}
}
