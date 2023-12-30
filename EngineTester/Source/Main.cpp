#include <Engine/Core/App.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Input/InputAdapter.hpp>
#include <Engine/Input/InputAction.hpp>

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

		const uint32_t deviceID = 6;

		auto walkAction = m_InputSystem->RegisterAction({
			.Type = AxisType::Axis2D,
			.AxisBindings = {
				{
					.TargetAxis = Axis::X,
					.Bindings = {
						{ { deviceID, 'W' }, 1.0f },
						{ { deviceID, 'S' }, -1.0f },
					}
				},
				{
					.TargetAxis = Axis::Y,
					.Bindings = {
						{ { deviceID, 'D' }, 1.0f },
						{ { deviceID, 'A' }, -1.0f },
					}
				}
			},
			.ConsumeInputs = true
		});

		context.BindAction(walkAction, [](InputReading reading)
		{
			const auto& value = reading.Read<AxisValue2D>();
			std::cout << "X: " << value.X << ", Y: " << value.Y << "\n";
		});

		InputContextID contextID = m_InputSystem->RegisterContext(context);

		m_InputSystem->ActivateContext(contextID);
	}

	void OnUpdate() override
	{
		/*const auto& value = m_Controller->ReadChannelValue('W').ReadValue<Yuki::AxisValue1D>();

		if (value.Value)
		{
			std::cout << "W is pressed!\n";
		}*/

		if (m_Window->IsClosed())
		{
			m_Running = false;
		}
	}

private:
	Yuki::Window* m_Window;
	const Yuki::InputDevice* m_Controller;

};

int main()
{
	Yuki::AppRunner<EngineTester>().Run();
	return 0;
}
