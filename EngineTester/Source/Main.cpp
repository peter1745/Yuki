#include <Engine/Core/App.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputContext.hpp>
#include <Engine/Input/InputAdapter.hpp>

#include <iostream>
#include <Windows.h>

class EngineTester final : public Yuki::Application
{
protected:
	void OnRun() override
	{
		m_Window = m_WindowSystem.NewWindow("Input Testing");

		Yuki::InputContext context;

		for (uint32_t i = 0; i < m_Dispatcher.GetDeviceCount(); i++)
		{
			const auto& device = m_Dispatcher.GetDevice(i);

			if (device.GetType() == Yuki::InputDevice::Type::Keyboard)
			{
				m_Controller = &device;
				break;
			}
		}

		/*m_InputSystem->RegisterTriggers("WalkForward",
		{
			{ InputID(Yuki::AnyDevice, InputCode::W), 1.0f },
			{ InputID(Yuki::AnyDevice, InputCode::S), -1.0f },
			{ InputID(m_Controller->GetID(), 0) }
		});

		context.AddAction({
			m_InputSystem->GetTriggers("WalkForward"),
			[](InputReading reading)
			{
				reading.Read<Axis2D>();
			}
		});*/
	}

	void OnUpdate() override
	{
		m_WindowSystem.PollEvents();
		m_Dispatcher.Update();

		const auto& value = m_Controller->ReadChannelValue('W').ReadValue<Yuki::AxisValue1D>();

		if (value.Value)
		{
			std::cout << "W is pressed!\n";
		}

		if (m_Window->IsClosed())
		{
			m_Running = false;
		}
	}

private:
	Yuki::WindowSystem m_WindowSystem;
	Yuki::Window* m_Window;
	Yuki::InputAdapter m_Dispatcher;
	const Yuki::InputDevice* m_Controller;

};

int main()
{
	Yuki::AppRunner<EngineTester>().Run();
	return 0;
}
