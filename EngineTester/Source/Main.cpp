#include <Engine/Core/EntryPoint.hpp>
#include <Engine/Core/Logging.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Input/InputAdapter.hpp>
#include <Engine/Input/InputAction.hpp>
#include <Engine/Input/InputCodes.hpp>

#include <Engine/RHI/RHI.hpp>

#include <iostream>
#include <ranges>

using namespace Yuki;

class EngineTester final : public Application
{
protected:
	void OnRun() override
	{
		m_Window = m_WindowSystem->NewWindow("Input Testing");

		for (auto i : std::views::iota(0, 50))
		{
			auto d = 0;
		}

		m_RHI = RHIContext::Create();

		auto ctx = m_InputSystem.CreateContext();

		using enum TriggerEventType;

		auto walkAction = m_InputSystem.RegisterAction({
			.AxisCount = 2,
			.AxisBindings = {
				{
					.Bindings = {
						{ { GenericKeyboard, KeyCode::D, OnPressed },  1.0f},
						{ { GenericKeyboard, KeyCode::A, OnPressed }, -1.0f},
					}
				},
				{
					.Bindings = {
						{ { GenericKeyboard, KeyCode::W, OnPressed | OnReleased },  1.0f},
						{ { GenericKeyboard, KeyCode::S, OnPressed }, -1.0f},
					}
				}
			},
			.ConsumeInputs = true
		});

		ctx.BindAction(walkAction, [](const auto)
		{
			WriteLine("Triggered");
		});

		ctx.Activate();
	}

	void OnUpdate() override
	{
		if (m_Window->IsClosed())
		{
			m_Running = false;
		}
	}

private:
	Window* m_Window;

	RHIContext m_RHI;
};

YukiApp(EngineTester) {};
