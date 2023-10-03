#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/RHI/Context.hpp>

#include <iostream>

class EngineTesterApp : public Yuki::Application
{
public:
	EngineTesterApp()
		: Yuki::Application()
	{
		using enum Yuki::RHI::RendererFeature;
		m_RHIContext = Yuki::RHI::Context::New({
			.RequestedFeatures = {
				RayTracing
			},
		});

		auto MyWindow = m_WindowSystem.NewWindow({
			.Title = "My Window"
		});

		m_GraphicsQueue = m_RHIContext->GetRenderDevice().QueueRequest(Yuki::RHI::QueueType::Graphics);

		m_Swapchain = m_RHIContext->GetRenderDevice().SwapchainCreate(m_WindowSystem, MyWindow);

		m_Fence = m_RHIContext->GetRenderDevice().FenceCreate();
		m_CommandPool = m_RHIContext->GetRenderDevice().CommandPoolCreate(m_GraphicsQueue);
	}

	void Update() override
	{
		m_WindowSystem.PollMessages();

		auto& RenderDevice = m_RHIContext->GetRenderDevice();

		RenderDevice.QueueAcquireImages(m_GraphicsQueue, { m_Swapchain }, { m_Fence });

		auto SwapchainImage = RenderDevice.SwapchainGetCurrentImage(m_Swapchain);

		Yuki::RHI::RenderTarget RenderTarget =
		{
			.ColorAttachments = {
				{
					.ImageView = RenderDevice.SwapchainGetCurrentImageView(m_Swapchain),
					.LoadOp = Yuki::RHI::AttachmentLoadOp::Clear,
					.StoreOp = Yuki::RHI::AttachmentStoreOp::Store
				}
			}
		};

		auto CmdList = RenderDevice.CommandPoolNewList(m_CommandPool);
		RenderDevice.CommandListBegin(CmdList);
		RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Attachment } });
		
		RenderDevice.CommandListBeginRendering(CmdList, RenderTarget);
		RenderDevice.CommandListEndRendering(CmdList);

		RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Present } });
		RenderDevice.CommandListEnd(CmdList);
		RenderDevice.QueueSubmit(m_GraphicsQueue, { CmdList }, { m_Fence }, { m_Fence });

		RenderDevice.QueuePresent(m_GraphicsQueue, { m_Swapchain }, { m_Fence });
	}

private:
	Yuki::Unique<Yuki::RHI::Context> m_RHIContext;
	Yuki::WindowSystem m_WindowSystem;
	Yuki::RHI::SwapchainRH m_Swapchain;
	Yuki::RHI::QueueRH m_GraphicsQueue;
	Yuki::RHI::FenceRH m_Fence;
	Yuki::RHI::CommandPoolRH m_CommandPool;
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)