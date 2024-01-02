#include <Engine/Core/App.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Input/InputAdapter.hpp>
#include <Engine/Input/InputAction.hpp>
#include <Engine/Input/InputCodes.hpp>

#include <iostream>
#include <ranges>

using namespace Yuki;

class EngineTester final : public Application
{
protected:
	void OnRun() override
	{
		m_Window = m_WindowSystem->NewWindow("Input Testing");

		m_Context = m_InputSystem.CreateContext();
		m_OtherContext = m_InputSystem.CreateContext();

		auto walkAction = m_InputSystem.RegisterAction({
			.ValueCount = 2,
			.AxisBindings = {
				{
					.Bindings = {
						{ { GenericKeyboard, KeyCode::D },  1.0f},
						{ { GenericKeyboard, KeyCode::A }, -1.0f},
					}
				},
				{
					.Bindings = {
						{ { GenericKeyboard, KeyCode::W },  1.0f},
						{ { GenericKeyboard, KeyCode::S }, -1.0f},
					}
				}
			},
			.ConsumeInputs = true
		});

		m_Context.BindAction(walkAction, [](const InputReading& reading)
		{
			auto [x, y] = reading.Read<2>();
			//std::cout << "X: " << x << ", Y: " << y << "\n";
			std::cout << "Main Context\n";
		});

		m_OtherContext.BindAction(walkAction, [](const InputReading& reading)
		{
			auto [x, y] = reading.Read<2>();
			std::cout << "Other Context\n";
		});

		auto mouseAction = m_InputSystem.RegisterAction({
			.ValueCount = 1,
			.AxisBindings = {
				{
					.Bindings = {
						{ { GenericMouse, MouseCode::WheelScrollY },   1.0f },
					}
				}
			},
			.ConsumeInputs = true
		});

		m_Context.BindAction(mouseAction, [&](const InputReading& reading)
		{
			auto [x] = reading.Read<1>();
			std::cout << "Scroll Delta: " << x << "\n";
		});

		m_Context.Activate();
		m_OtherContext.Activate();
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
	InputContext m_Context, m_OtherContext;

};

int main()
{
	AppRunner<EngineTester>().Run();
	return 0;
}
