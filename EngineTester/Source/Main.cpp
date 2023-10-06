#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/RHI/Context.hpp>
#include <Engine/Math/Mat4.hpp>

#include <iostream>

struct PushConstants
{
	uint64_t TopLevelAS;
	Yuki::Math::Vec3 ViewPos;
	Yuki::Math::Vec3 CameraX;
	Yuki::Math::Vec3 CameraY;
	float CameraZOffset;
} c_PushConstants;

struct Vertex
{
	float Position[3];
};

class EngineTesterApp : public Yuki::Application
{
public:
	EngineTesterApp()
		: Yuki::Application()
	{
		using enum Yuki::RHI::RendererFeature;
		m_RHIContext = Yuki::RHI::Context::New({
			.RequestedFeatures = {
				RayTracing,
			},
		});

		m_Window = m_WindowSystem.NewWindow({
			.Title = "My Window"
		});

		auto& RenderDevice = m_RHIContext->GetRenderDevice();

		m_GraphicsQueue = RenderDevice.QueueRequest(Yuki::RHI::QueueType::Graphics);

		m_Swapchain = RenderDevice.SwapchainCreate(m_WindowSystem, m_Window);

		m_Fence = RenderDevice.FenceCreate();
		m_CommandPool = RenderDevice.CommandPoolCreate(m_GraphicsQueue);

		std::array Vertices =
		{
			 -0.5f, 0.5f, 0.0f,
			 0.5f, 0.5f, 0.0f,
			 0.0f, -0.5f, 0.0f
		};

		std::array Indices =
		{
			0U, 1U, 2U
		};

		auto StagingBuffer = RenderDevice.BufferCreate(3 * 3 * sizeof(float), Yuki::RHI::BufferUsage::TransferSrc, true);
		RenderDevice.BufferSetData(StagingBuffer, Vertices.data());

		m_VertexBuffer = RenderDevice.BufferCreate(3 * 3 * sizeof(float), Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);
		m_IndexBuffer = RenderDevice.BufferCreate(3 * sizeof(uint32_t), Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);

		auto CmdList = RenderDevice.CommandPoolNewList(m_CommandPool);
		RenderDevice.CommandListBegin(CmdList);
		RenderDevice.CommandListCopyBuffer(CmdList, m_VertexBuffer, StagingBuffer);
		RenderDevice.CommandListEnd(CmdList);
		RenderDevice.QueueSubmit(m_GraphicsQueue, { CmdList }, {}, { m_Fence });

		RenderDevice.FenceWait(m_Fence);

		RenderDevice.BufferSetData(StagingBuffer, Indices.data());

		CmdList = RenderDevice.CommandPoolNewList(m_CommandPool);
		RenderDevice.CommandListBegin(CmdList);
		RenderDevice.CommandListCopyBuffer(CmdList, m_IndexBuffer, StagingBuffer);
		RenderDevice.CommandListEnd(CmdList);
		RenderDevice.QueueSubmit(m_GraphicsQueue, { CmdList }, {}, { m_Fence });

		RenderDevice.FenceWait(m_Fence);

		RenderDevice.BufferDestroy(StagingBuffer);

		m_AccelerationStructure = RenderDevice.AccelerationStructureCreate(m_VertexBuffer, m_IndexBuffer);

		m_DescriptorLayout = RenderDevice.DescriptorSetLayoutCreate({
			.Stages = Yuki::RHI::ShaderStage::RayGeneration,
			.Descriptors = {
				{ 1, Yuki::RHI::DescriptorType::StorageImage }
			}
		});

		Yuki::RHI::DescriptorCount DC = { Yuki::RHI::DescriptorType::StorageImage, 1 };
		m_DescriptorPool = RenderDevice.DescriptorPoolCreate({ DC });
		m_DescriptorSet = RenderDevice.DescriptorPoolAllocateDescriptorSet(m_DescriptorPool, m_DescriptorLayout);

		m_Pipeline = RenderDevice.PipelineCreate({
			.Type = Yuki::RHI::PipelineType::Raytracing,
			.Shaders = {
				{ "Shaders/Raytracing.glsl", Yuki::RHI::ShaderStage::RayGeneration },
				{ "Shaders/Raytracing.glsl", Yuki::RHI::ShaderStage::RayMiss },
				{ "Shaders/Raytracing.glsl", Yuki::RHI::ShaderStage::RayClosestHit },
			},
			.PushConstantSize = sizeof(PushConstants),
			.DescriptorLayouts = {
				m_DescriptorLayout
			}
		});

		c_PushConstants.CameraX = { 1.0f, 0.0f, 0.0f };
		c_PushConstants.CameraY = { 0.0f, 1.0f, 0.0f };
		c_PushConstants.CameraZOffset = 1.0f / std::tanf(0.5f * 1.22173048f);
	}

	void Update() override
	{
		m_WindowSystem.PollMessages();

		auto& RenderDevice = m_RHIContext->GetRenderDevice();

		RenderDevice.FenceWait(m_Fence);

		RenderDevice.QueueAcquireImages(m_GraphicsQueue, { m_Swapchain }, { m_Fence });

		auto SwapchainImage = RenderDevice.SwapchainGetCurrentImage(m_Swapchain);

		/*Yuki::RHI::RenderTarget RenderTarget =
		{
			.ColorAttachments = {
				{
					.ImageView = RenderDevice.SwapchainGetCurrentImageView(m_Swapchain),
					.LoadOp = Yuki::RHI::AttachmentLoadOp::Clear,
					.StoreOp = Yuki::RHI::AttachmentStoreOp::Store
				}
			}
		};*/

		static float t = 0.0f;
		t += Yuki::EngineTime::DeltaTime();
		c_PushConstants.ViewPos = { std::cosf(t) * 5.0f, 0.0f, 10.0f + std::sinf(t) * 5.0f };

		RenderDevice.DescriptorSetWrite(m_DescriptorSet, 0, { RenderDevice.SwapchainGetCurrentImageView(m_Swapchain) }, 0);

		RenderDevice.CommandPoolReset(m_CommandPool);

		auto CmdList = RenderDevice.CommandPoolNewList(m_CommandPool);
		RenderDevice.CommandListBegin(CmdList);
		RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::General } });
		
		c_PushConstants.TopLevelAS = RenderDevice.AccelerationStructureGetTopLevelAddress(m_AccelerationStructure);
		RenderDevice.CommandListPushConstants(CmdList, m_Pipeline, Yuki::RHI::ShaderStage::RayGeneration, &c_PushConstants, sizeof(c_PushConstants));

		RenderDevice.CommandListBindDescriptorSets(CmdList, m_Pipeline, { m_DescriptorSet });
		RenderDevice.CommandListBindPipeline(CmdList, m_Pipeline);

		const auto& WindowData = m_WindowSystem.GetWindowData(m_Window);

		RenderDevice.CommandListTraceRay(CmdList, m_Pipeline, WindowData.Width, WindowData.Height);

		RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Present } });
		RenderDevice.CommandListEnd(CmdList);
		RenderDevice.QueueSubmit(m_GraphicsQueue, { CmdList }, { m_Fence }, { m_Fence });

		RenderDevice.QueuePresent(m_GraphicsQueue, { m_Swapchain }, { m_Fence });
	}

private:
	Yuki::Unique<Yuki::RHI::Context> m_RHIContext;
	Yuki::WindowSystem m_WindowSystem;
	Yuki::WindowHandle m_Window;
	Yuki::RHI::SwapchainRH m_Swapchain;
	Yuki::RHI::QueueRH m_GraphicsQueue;
	Yuki::RHI::FenceRH m_Fence;
	Yuki::RHI::CommandPoolRH m_CommandPool;
	Yuki::RHI::BufferRH m_VertexBuffer;
	Yuki::RHI::BufferRH m_IndexBuffer;
	Yuki::RHI::PipelineRH m_Pipeline;
	Yuki::RHI::DescriptorSetLayoutRH m_DescriptorLayout;
	Yuki::RHI::DescriptorPoolRH m_DescriptorPool;
	Yuki::RHI::DescriptorSetRH m_DescriptorSet;
	Yuki::RHI::AccelerationStructureRH m_AccelerationStructure;
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)