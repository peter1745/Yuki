#include <Engine/Core/App.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Input/InputAdapter.hpp>
#include <Engine/Input/InputAction.hpp>
#include <Engine/Input/InputCodes.hpp>

#include <iostream>
#include <ranges>
#include <Windows.h>

class EngineTester final : public Yuki::Application
{
protected:
	void OnRun() override
	{
		m_Window = m_WindowSystem->NewWindow("Input Testing");

		using namespace Yuki;

		InputContext context;

		/*
		TODO(Peter):
			- Implement InputID detection (e.g press thumbstick forward and get the input id for that device + input)
			- Add support for Pressed / Released / Held behavior for triggers
			- Input Mixers
		*/

		auto walkAction = m_InputSystem->RegisterAction({
			.Type = AxisType::Axis2D,
			.AxisBindings = {
				{
					.TargetAxis = Axis::X,
					.Bindings = {
						{ { AnyKeyboardDevice, KeyCode::D },  1.0f},
						{ { AnyKeyboardDevice, KeyCode::A }, -1.0f},
					}
				},
				{
					.TargetAxis = Axis::Y,
					.Bindings = {
						{ { AnyKeyboardDevice, KeyCode::W },  1.0f},
						{ { AnyKeyboardDevice, KeyCode::S }, -1.0f},
					}
				}
			},
			.ConsumeInputs = true
		});

		/*auto driveAction = m_InputSystem->RegisterAction({
			.Type = AxisType::Axis2D,
			.AxisBindings = {
				{
					.TargetAxis = Axis::X,
					.Bindings = {
						{ { deviceID, 'D'}, 1.0f },
						{ { deviceID, 'A' }, -1.0f },
					}
				},
				{
					.TargetAxis = Axis::Y,
					.Bindings = {
						{ { deviceID, 'W' }, 1.0f },
						{ { deviceID, 'S' }, -1.0f },
						{ { deviceID, 'K' }, -1.0f },
						{{ 0, 6 }, 1.0f }
					}
				},
			},
			.ConsumeInputs = true
		});*/

		context.BindAction(walkAction, [](InputReading reading)
		{
			const auto& value = reading.Read<AxisValue2D>();
			std::cout << "X: " << value.X << ", Y: " << value.Y << "\n";
		});

		/*context.BindAction(driveAction, [](InputReading reading)
		{
			const auto& value = reading.Read<AxisValue2D>();
			std::cout << "DRIVE ACTION\n";
		});*/

		InputContextID contextID = m_InputSystem->RegisterContext(context);

		m_InputSystem->ActivateContext(contextID);
	}

	void OnUpdate() override
	{
		if (m_Window->IsClosed())
		{
			m_Running = false;
		}
	}

private:
	Yuki::Window* m_Window;
};

int main()
{
	Yuki::AppRunner<EngineTester>().Run();
	return 0;
}
