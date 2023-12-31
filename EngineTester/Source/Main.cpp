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
						{ { AnyKeyboardDevice, KeyCode::D },  1.0f},
						{ { AnyKeyboardDevice, KeyCode::A }, -1.0f},
					}
				},
				{
					.Bindings = {
						{ { AnyKeyboardDevice, KeyCode::W },  1.0f},
						{ { AnyKeyboardDevice, KeyCode::S }, -1.0f},
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
						{ { AnyMouseDevice, MouseCode::ButtonLeft },     1.0f },
						{ { AnyMouseDevice, MouseCode::ButtonRight },    2.0f },
						{ { AnyMouseDevice, MouseCode::ButtonMiddle },   3.0f },
						{ { AnyMouseDevice, MouseCode::Button4 },        4.0f },
						{ { AnyMouseDevice, MouseCode::Button5 },        5.0f },
						{ { AnyMouseDevice, MouseCode::WheelTiltLeft },  6.0f },
						{ { AnyMouseDevice, MouseCode::WheelTiltRight }, 7.0f },
						{ { AnyMouseDevice, MouseCode::WheelScrollX },   8.0f },
						{ { AnyMouseDevice, MouseCode::WheelScrollY },   9.0f },
					}
				}
			},
			.ConsumeInputs = true
		});

		m_ContextID = m_InputSystem->CreateContext();

		m_InputSystem->BindAction(m_ContextID, walkAction, [&](const InputReading& reading)
		{
			auto [x, y] = reading.Read<2>();
			std::cout << "X: " << x << ", Y: " << y << "\n";
		});

		m_InputSystem->BindAction(m_ContextID, mouseAction, [&](const InputReading& reading)
		{
			auto [x] = reading.Read<1>();
			std::cout << "Mouse Button: " << x << "\n";
		});

		m_InputSystem->ActivateContext(m_ContextID);
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
	InputContextID m_ContextID;

};

int main()
{
	AppRunner<EngineTester>().Run();
	return 0;
}
