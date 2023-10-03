#include "Application.hpp"

#include "EngineTime.hpp"
#include "Engine/Messages/EngineMessages.hpp"

namespace Yuki {

	void Application::Run()
	{
		EngineMessages::Get().AddListener<WindowCloseMessage>([this](const auto& InMessage)
		{
			ShouldClose = InMessage.IsPrimaryWindow;
		});

		while (!ShouldClose)
		{
			EngineTime::CalculateDeltaTime();

			Update();

			EngineMessages::Get().ProcessMessages();
		}
	}
}
