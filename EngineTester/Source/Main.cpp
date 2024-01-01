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

		auto walkAction = m_InputSystem->RegisterAction({
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

		auto mouseAction = m_InputSystem->RegisterAction({
			.ValueCount = 1,
			.AxisBindings = {
				{
					.Bindings = {
						{ { GenericMouse, MouseCode::ButtonLeft },     1.0f },
						{ { GenericMouse, MouseCode::ButtonRight },    2.0f },
						{ { GenericMouse, MouseCode::ButtonMiddle },   3.0f },
						{ { GenericMouse, MouseCode::Button4 },        4.0f },
						{ { GenericMouse, MouseCode::Button5 },        5.0f },
						{ { GenericMouse, MouseCode::WheelTiltLeft },  6.0f },
						{ { GenericMouse, MouseCode::WheelTiltRight }, 7.0f },
						{ { GenericMouse, MouseCode::WheelScrollX },   8.0f },
						{ { GenericMouse, MouseCode::WheelScrollY },   9.0f },
					}
				}
			},
			.ConsumeInputs = true
		});

		m_ContextID = m_InputSystem->CreateContext();
		m_OtherContext = m_InputSystem->CreateContext();

		m_InputSystem->BindAction(m_ContextID, walkAction, [&](const InputReading& reading)
		{
			auto [x, y] = reading.Read<2>();
			//std::cout << "X: " << x << ", Y: " << y << "\n";
			std::cout << "Main Context\n";
		});

		m_InputSystem->BindAction(m_OtherContext, walkAction, [&](const InputReading& reading)
		{
			auto [x, y] = reading.Read<2>();
			std::cout << "Other Context\n";
		});

		m_InputSystem->BindAction(m_ContextID, mouseAction, [&](const InputReading& reading)
		{
			auto [x] = reading.Read<1>();
			std::cout << "Mouse Button: " << x << "\n";
		});

		m_InputSystem->ActivateContext(m_ContextID);
		m_InputSystem->ActivateContext(m_OtherContext);
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
	InputContextID m_ContextID, m_OtherContext;

};

int main()
{
	AppRunner<EngineTester>().Run();
	return 0;
}
