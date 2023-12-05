export module EngineTester;

import Yuki.Core;
import Yuki.Rendering;

class EngineTester final : public Yuki::Application
{
protected:
	void OnRun() override
	{
		m_Window = m_WindowSystem.NewWindow("EngineTester", 1920, 1080);

		m_RHIContext = Yuki::RHIContext::Create();

		const auto x = R"hlsl(
			struct VertexData
			{
				float3 Position : POSITION;
				float3 Color : COLOR;
			};

			struct VertexOutput
			{
				float4 Position : SV_Position;
				float3 Color : COLOR;
			};

			VertexOutput VSMain(VertexData vertexData)
			{
				VertexOutput output;
				output.Position = float4(vertexData.Position, 1.0f);
				output.Color = vertexData.Color;
				return output;
			}

			struct PixelInput
			{
				float3 Color : COLOR;
			};

			float4 PSMain(PixelInput input)
			{
				return float4(input.Color, 1.0f);
			}
		)hlsl";

		m_ShaderLibrary = Yuki::ShaderLibrary::Create(m_RHIContext);
		m_ShaderLibrary.Compile("Resources/HLSL/Triangle.hlsl");

		m_GraphicsQueue = m_RHIContext.RequestQueue(Yuki::QueueType::Graphics);
		m_Fence = Yuki::Fence::Create(m_RHIContext);

		m_Swapchain = Yuki::Swapchain::Create(m_RHIContext, m_GraphicsQueue, m_Window);
		m_CommandAllocator = Yuki::CommandAllocator::Create(m_RHIContext);
	}

	void OnUpdate() override
	{
		m_WindowSystem.PollEvents();

		m_Fence.Wait();

		m_CommandAllocator.Reset();
		auto list = m_CommandAllocator.NewList();

		m_GraphicsQueue.ExecuteCommandLists({ list }, { m_Fence });

		m_Swapchain.Present();

		m_Running = !m_Window.IsClosed();
	}

private:
	Yuki::WindowSystem m_WindowSystem;
	Yuki::Window m_Window;
	Yuki::RHIContext m_RHIContext;
	Yuki::ShaderLibrary m_ShaderLibrary;
	Yuki::Queue m_GraphicsQueue;
	Yuki::Fence m_Fence;
	Yuki::Swapchain m_Swapchain;
	Yuki::CommandAllocator m_CommandAllocator;
};

export int main()
{
	Yuki::AppRunner<EngineTester>().Run();
	return 0;
}
