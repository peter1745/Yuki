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

		m_RHI = RHIContext::Create();

		/*
		m_Queue = m_RHI.RequestQueue(CommandListType::Graphics);
		m_Swapchain = Swapchain::Create(m_RHI, m_Queue, m_Window);

		m_Shader = Shader::Create("Resources/HLSL/RayGen.hlsl", ShaderType::RayGeneration);
		m_RTPipeline = RaytracingPipeline::Create(m_RHI, m_Shader);*/

		//m_CommandAllocator = CommandAllocator::Create(m_RHI, CommandListType::Graphics);

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
		/*m_CommandAllocator.Reset();

		auto cmd = m_CommandAllocator.NewList();
		cmd.BindPipeline(m_RTPipeline);
		cmd.DispatchRays(1920, 1080);

		m_Queue.SubmitCommandLists({ cmd });

		m_Swapchain.Present();*/

		if (m_Window.IsClosed())
		{
			m_Running = false;
		}
	}

	void OnShutdown() override
	{
		m_RHI.Destroy();
	}

private:
	Window m_Window;

	RHIContext m_RHI;
	/*Queue m_Queue;
	Swapchain m_Swapchain;

	CommandAllocator m_CommandAllocator;

	Shader m_Shader;
	RaytracingPipeline m_RTPipeline;*/
};

YukiApp(EngineTester) {};
