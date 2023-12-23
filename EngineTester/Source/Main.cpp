#include <Engine/Core/App.hpp>
#include <Engine/Core/Window.hpp>
#include <Engine/Input/InputContext.hpp>
#include <Engine/Input/InputDisptacher.hpp>

#include <iostream>

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

			if (device.GetType() == Yuki::InputDevice::Type::Controller)
			{
				m_Controller = &device;
				break;
			}
		}

		/*
			ChannelID forwardChannel = AddChannel([]()
			{
				
			}).BindDeviceChannel(device, );

			ChannelID rightChannel = AddChannel([]()
			{
				
			}).BindInput();
		
			context.AddAction([](const ActionData& actionData)
			{
				camera.Translation += actionData.ReadAxis(Axis::X);
				camera.Translation += actionData.ReadAxis(Axis::Y);
			}).BindChannel(id);

			drivingContext.AddAction([](const ActionData& actionData)
			{
				actionData.ReadChannel();
				car.Turn();
			}).BindChannel(id);

			m_InputSystem.SetActiveContext(m_InputSystem.AddContext(context));
		*/
	}

	void OnUpdate() override
	{
		m_WindowSystem.PollEvents();
		m_Dispatcher.Update();

		std::cout << "Channel 0: " << m_Controller->ReadChannelValue(1) << "\n";

		if (m_Window->IsClosed())
		{
			m_Running = false;
		}
	}

private:
	Yuki::WindowSystem m_WindowSystem;
	Yuki::Window* m_Window;
	Yuki::InputDispatcher m_Dispatcher;
	const Yuki::InputDevice* m_Controller;

};

int main()
{
	Yuki::AppRunner<EngineTester>().Run();
	return 0;
}
