#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/RHI/Context.hpp>
#include <Engine/Input/InputContext.hpp>

#include <iostream>

struct PushConstants
{
	uint64_t TopLevelAS;
	Yuki::Vec3 ViewPos = { 0.0f, 0.0f, 10.0f };
	Yuki::Vec3 CameraX;
	Yuki::Vec3 CameraY;
	float CameraZOffset;
} c_PushConstants;

struct RasterPushConstants
{
	uint64_t VertexBuffer;
	glm::mat4 ViewProjection;
} c_RasterPushConstants;

struct Vertex
{
	Yuki::Vec3 Position;
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
			Vertex{ { -0.5f, 0.5f, 0.0f } },
			Vertex{ { 0.5f, 0.5f,  0.0f } },
			Vertex{ { 0.0f, -0.5f, 0.0f } }
		};

		std::array Indices =
		{
			0U, 1U, 2U
		};

		auto StagingBuffer = RenderDevice.BufferCreate(Vertices.size() * sizeof(Vertex), Yuki::RHI::BufferUsage::TransferSrc, true);
		RenderDevice.BufferSetData(StagingBuffer, Vertices.data());

		m_VertexBuffer = RenderDevice.BufferCreate(Vertices.size() * sizeof(Vertex), Yuki::RHI::BufferUsage::Storage | Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);
		m_IndexBuffer = RenderDevice.BufferCreate(3 * sizeof(uint32_t), Yuki::RHI::BufferUsage::Index | Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);

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
				{ 1, Yuki::RHI::DescriptorType::StorageImage },
			}
		});

		Yuki::RHI::DescriptorCount DC = { Yuki::RHI::DescriptorType::StorageImage, 1 };
		m_DescriptorPool = RenderDevice.DescriptorPoolCreate({ DC });
		m_DescriptorSet = RenderDevice.DescriptorPoolAllocateDescriptorSet(m_DescriptorPool, m_DescriptorLayout);

		m_Pipeline = RenderDevice.RayTracingPipelineCreate({
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

		m_RasterDescriptorLayout = RenderDevice.DescriptorSetLayoutCreate({
			.Stages = Yuki::RHI::ShaderStage::Vertex | Yuki::RHI::ShaderStage::Fragment
		});

		m_RasterizationPipeline = RenderDevice.PipelineCreate({
			.Shaders = {
				{ "Shaders/Raster.glsl", Yuki::RHI::ShaderStage::Vertex },
				{ "Shaders/Raster.glsl", Yuki::RHI::ShaderStage::Fragment },
			},
			.PushConstantSize = sizeof(RasterPushConstants),
			.ColorAttachments = {
				{ Yuki::RHI::ImageFormat::BGRA8 }
			}
		});

		c_PushConstants.CameraZOffset = 1.0f / std::tanf(0.5f * 1.22173048f);

		/*m_CameraInput.Bind<Yuki::RangedInput>(Yuki::RangedInput{
			{
				.Value = -1.0f,
				.Key = Yuki::KeyCode::A,
			},
			{
				.Value = 1.0f,
				.Key = Yuki::KeyCode::D,
			},
			[&](float InValue)
			{
				c_PushConstants.ViewPos.X += InValue * Yuki::EngineTime::DeltaTime();
			}
		});

		m_CameraInput.Bind<Yuki::RangedInput>({
			{
				.Value = -1.0f,
				.Key = Yuki::KeyCode::W,
			},
			{
				.Value = 1.0f,
				.Key = Yuki::KeyCode::S,
			},
			[&](float InValue)
			{
				c_PushConstants.ViewPos.Z += InValue * 10.0f * Yuki::EngineTime::DeltaTime();
			}
		});*/

		//m_WindowSystem.AddInputContext(m_Window, &m_CameraInput);
	}

	inline static bool s_RayTrace = true;

	void Update() override
	{
		m_WindowSystem.PollMessages();

		auto& RenderDevice = m_RHIContext->GetRenderDevice();

		RenderDevice.FenceWait(m_Fence);

		RenderDevice.QueueAcquireImages(m_GraphicsQueue, { m_Swapchain }, { m_Fence });

		auto SwapchainImage = RenderDevice.SwapchainGetCurrentImage(m_Swapchain);

		Yuki::Vec3 Translation = {};

		if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::W))
		{
			Translation.z -= 5.0f;
		}
		else if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::S))
		{
			Translation.z += 5.0f;
		}

		if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::A))
		{
			Translation.x -= 5.0f;
		}
		else if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::D))
		{
			Translation.x += 5.0f;
		}

		if (m_WindowSystem.GetMouseButtonState(m_Window, Yuki::MouseButton::Right))
		{
			m_WindowSystem.SetCursorLock(m_Window, true);

			int32_t DeltaX = m_WindowSystem.GetRawMouseDeltaX(m_Window) * -1;
			int32_t DeltaY = m_WindowSystem.GetRawMouseDeltaY(m_Window) * -1;

			m_Rotation = glm::angleAxis(DeltaX * 0.001f, Yuki::Vec3{ 0.0f, 1.0f, 0.0f }) * m_Rotation;
			auto PitchedRotation = m_Rotation * glm::angleAxis(DeltaY * 0.001f, Yuki::Vec3{ -1.0f, 0.0f, 0.0f });
			if (glm::dot(PitchedRotation * Yuki::Vec3{ 0.0f, 1.0f, 0.0f }, Yuki::Vec3{ 0.0f, 1.0f, 0.0f }) >= 0.0f)
			{
				m_Rotation = PitchedRotation;
			}
			m_Rotation = glm::normalize(m_Rotation);
		}
		else
		{
			m_WindowSystem.SetCursorLock(m_Window, false);
		}

		const auto& WindowData = m_WindowSystem.GetWindowData(m_Window);

		RenderDevice.CommandPoolReset(m_CommandPool);
		auto CmdList = RenderDevice.CommandPoolNewList(m_CommandPool);
		RenderDevice.CommandListBegin(CmdList);

		if (s_RayTrace)
		{
			c_PushConstants.ViewPos += (m_Rotation * Translation) * Yuki::EngineTime::DeltaTime<float>();
			c_PushConstants.CameraX = m_Rotation * Yuki::Vec3{ 1.0f, 0.0f, 0.0f };
			c_PushConstants.CameraY = m_Rotation * Yuki::Vec3{ 0.0f, 1.0f, 0.0f };

			RenderDevice.DescriptorSetWrite(m_DescriptorSet, 0, { RenderDevice.SwapchainGetCurrentImageView(m_Swapchain) }, 0);

			RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::General } });

			c_PushConstants.TopLevelAS = RenderDevice.AccelerationStructureGetTopLevelAddress(m_AccelerationStructure);
			RenderDevice.CommandListPushConstants(CmdList, m_Pipeline, Yuki::RHI::ShaderStage::RayGeneration, &c_PushConstants, sizeof(c_PushConstants));

			RenderDevice.CommandListBindDescriptorSets(CmdList, m_Pipeline, { m_DescriptorSet });
			RenderDevice.CommandListBindPipeline(CmdList, m_Pipeline);

			RenderDevice.CommandListTraceRay(CmdList, m_Pipeline, WindowData.Width, WindowData.Height);
		}
		else
		{
			static Yuki::Vec3 Position;
			Position += (m_Rotation * Translation) * Yuki::EngineTime::DeltaTime<float>();
			c_RasterPushConstants.VertexBuffer = RenderDevice.BufferGetDeviceAddress(m_VertexBuffer);
			c_RasterPushConstants.ViewProjection = Yuki::PerspectiveInfReversedZ(glm::radians(90.0f), Yuki::Cast<float>(WindowData.Width) / Yuki::Cast<float>(WindowData.Height), 0.001f)
				* glm::inverse(glm::translate(glm::mat4(1.0f), Position) * glm::toMat4(m_Rotation));

			RenderDevice.CommandListImageBarrier(CmdList, { .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Attachment } });
			RenderDevice.CommandListPushConstants(CmdList, m_RasterizationPipeline, Yuki::RHI::ShaderStage::Vertex, &c_RasterPushConstants, sizeof(c_RasterPushConstants));
			RenderDevice.CommandListBindPipeline(CmdList, m_RasterizationPipeline);
			RenderDevice.CommandListBindIndexBuffer(CmdList, m_IndexBuffer);
			RenderDevice.CommandListSetViewport(CmdList, {
				.X = 0.0f,
				.Y = 0.0f,
				.Width = Yuki::Cast<float>(WindowData.Width),
				.Height = Yuki::Cast<float>(WindowData.Height),
			});

			RenderDevice.CommandListBeginRendering(CmdList, {
				.ColorAttachments = {
					{
						.ImageView = RenderDevice.SwapchainGetCurrentImageView(m_Swapchain),
						.LoadOp = Yuki::RHI::AttachmentLoadOp::Clear,
						.StoreOp = Yuki::RHI::AttachmentStoreOp::Store
					}
				}
			});

			RenderDevice.CommandListDrawIndexed(CmdList, 3, 0, 0);
			RenderDevice.CommandListEndRendering(CmdList);
		}

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
	Yuki::RHI::RayTracingPipelineRH m_Pipeline;
	Yuki::RHI::DescriptorSetLayoutRH m_DescriptorLayout;
	Yuki::RHI::DescriptorPoolRH m_DescriptorPool;
	Yuki::RHI::DescriptorSetRH m_DescriptorSet;
	Yuki::RHI::AccelerationStructureRH m_AccelerationStructure;

	Yuki::RHI::PipelineRH m_RasterizationPipeline;
	Yuki::RHI::DescriptorSetLayoutRH m_RasterDescriptorLayout;
	Yuki::RHI::DescriptorPoolRH m_RasterDescriptorPool;
	Yuki::RHI::DescriptorSetRH m_RasterDescriptorSet;

	Yuki::InputContext m_CameraInput;

	Yuki::Quat m_Rotation{1.0f, 0.0f, 0.0f, 0.0f};
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)