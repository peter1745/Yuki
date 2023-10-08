#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/RHI/RenderHandles.hpp>
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
		m_RHIContext = Yuki::RHI::Context::Create({
			.RequestedFeatures = {
				RayTracing,
			},
		});

		m_Window = m_WindowSystem.NewWindow({
			.Title = "My Window"
		});

			m_GraphicsQueue = m_RHIContext.RequestQueue(Yuki::RHI::QueueType::Graphics);

		m_Swapchain = Yuki::RHI::Swapchain::Create(m_RHIContext, m_WindowSystem, m_Window);

		m_Fence = Yuki::RHI::Fence::Create(m_RHIContext);
		m_CommandPool = Yuki::RHI::CommandPool::Create(m_RHIContext, m_GraphicsQueue);

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

		auto StagingBuffer = Yuki::RHI::Buffer::Create(m_RHIContext, Vertices.size() * sizeof(Vertex), Yuki::RHI::BufferUsage::TransferSrc, true);
		StagingBuffer.SetData(Vertices.data());

		m_VertexBuffer = Yuki::RHI::Buffer::Create(m_RHIContext, Vertices.size() * sizeof(Vertex), Yuki::RHI::BufferUsage::Storage | Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);
		m_IndexBuffer = Yuki::RHI::Buffer::Create(m_RHIContext, 3 * sizeof(uint32_t), Yuki::RHI::BufferUsage::Index | Yuki::RHI::BufferUsage::AccelerationStructureBuildInput | Yuki::RHI::BufferUsage::TransferDst);

		auto CmdList = m_CommandPool.NewList();
		CmdList.Begin();
		CmdList.CopyBuffer(m_VertexBuffer, StagingBuffer);
		CmdList.End();
		m_GraphicsQueue.Submit({ CmdList }, {}, { m_Fence });

		m_Fence.Wait();

		StagingBuffer.SetData(Indices.data());

		CmdList = m_CommandPool.NewList();
		CmdList.Begin();
		CmdList.CopyBuffer(m_IndexBuffer, StagingBuffer);
		CmdList.End();
		m_GraphicsQueue.Submit({ CmdList }, {}, { m_Fence });

		m_Fence.Wait();

		//StagingBuffer.Destroy();

		m_AccelerationStructure = Yuki::RHI::AccelerationStructure::Create(m_RHIContext, m_VertexBuffer, m_IndexBuffer);

		m_DescriptorLayout = Yuki::RHI::DescriptorSetLayout::Create(m_RHIContext, {
			.Stages = Yuki::RHI::ShaderStage::RayGeneration,
			.Descriptors = {
				{ 1, Yuki::RHI::DescriptorType::StorageImage },
			}
		});

		Yuki::RHI::DescriptorCount DC = { Yuki::RHI::DescriptorType::StorageImage, 1 };
		m_DescriptorPool = Yuki::RHI::DescriptorPool::Create(m_RHIContext, { DC });
		m_DescriptorSet = m_DescriptorPool.AllocateDescriptorSet(m_DescriptorLayout);

		m_Pipeline = Yuki::RHI::RayTracingPipeline::Create(m_RHIContext, {
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

		m_RasterDescriptorLayout = Yuki::RHI::DescriptorSetLayout::Create(m_RHIContext, {
			.Stages = Yuki::RHI::ShaderStage::Vertex | Yuki::RHI::ShaderStage::Fragment
		});

		m_RasterizationPipeline = Yuki::RHI::Pipeline::Create(m_RHIContext, {
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

	inline static bool s_RayTrace = false;

	void Update() override
	{
		m_WindowSystem.PollMessages();

		m_Fence.Wait();
		
		m_GraphicsQueue.AcquireImages({ m_Swapchain }, { m_Fence });

		auto SwapchainImage = m_Swapchain.GetCurrentImage();

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

		m_CommandPool.Reset();
		auto CmdList = m_CommandPool.NewList();
		CmdList.Begin();

		if (s_RayTrace)
		{
			c_PushConstants.ViewPos += (m_Rotation * Translation) * Yuki::EngineTime::DeltaTime<float>();
			c_PushConstants.CameraX = m_Rotation * Yuki::Vec3{ 1.0f, 0.0f, 0.0f };
			c_PushConstants.CameraY = m_Rotation * Yuki::Vec3{ 0.0f, 1.0f, 0.0f };

			m_DescriptorSet.Write(0, { m_Swapchain.GetCurrentImageView() }, 0);

			CmdList.ImageBarrier({ .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::General } });

			c_PushConstants.TopLevelAS = m_AccelerationStructure.GetTopLevelAddress();
			CmdList.PushConstants(m_Pipeline, Yuki::RHI::ShaderStage::RayGeneration, &c_PushConstants, sizeof(c_PushConstants));

			CmdList.BindDescriptorSets(m_Pipeline, { m_DescriptorSet });
			CmdList.BindPipeline(m_Pipeline);

			CmdList.TraceRay(m_Pipeline, WindowData.Width, WindowData.Height);
		}
		else
		{
			static Yuki::Vec3 Position;
			Position += (m_Rotation * Translation) * Yuki::EngineTime::DeltaTime<float>();
			c_RasterPushConstants.VertexBuffer = m_VertexBuffer.GetDeviceAddress();
			c_RasterPushConstants.ViewProjection = Yuki::PerspectiveInfReversedZ(glm::radians(90.0f), Yuki::Cast<float>(WindowData.Width) / Yuki::Cast<float>(WindowData.Height), 0.001f)
				* glm::inverse(glm::translate(glm::mat4(1.0f), Position) * glm::toMat4(m_Rotation));

			CmdList.ImageBarrier({ .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Attachment } });
			CmdList.PushConstants(m_RasterizationPipeline, Yuki::RHI::ShaderStage::Vertex, &c_RasterPushConstants, sizeof(c_RasterPushConstants));
			CmdList.BindPipeline(m_RasterizationPipeline);
			CmdList.BindIndexBuffer(m_IndexBuffer);
			CmdList.SetViewport({
				.X = 0.0f,
				.Y = 0.0f,
				.Width = Yuki::Cast<float>(WindowData.Width),
				.Height = Yuki::Cast<float>(WindowData.Height),
			});

			CmdList.BeginRendering({
				.ColorAttachments = {
					{
						.ImageView = m_Swapchain.GetCurrentImageView(),
						.LoadOp = Yuki::RHI::AttachmentLoadOp::Clear,
						.StoreOp = Yuki::RHI::AttachmentStoreOp::Store
					}
				}
			});

			CmdList.DrawIndexed(3, 0, 0);
			CmdList.EndRendering();
		}

		CmdList.ImageBarrier({ .Images = { SwapchainImage }, .Layouts = { Yuki::RHI::ImageLayout::Present } });
		CmdList.End();
		m_GraphicsQueue.Submit({ CmdList }, { m_Fence }, { m_Fence });
		m_GraphicsQueue.Present({ m_Swapchain }, { m_Fence });
	}

private:
	Yuki::RHI::Context m_RHIContext;
	Yuki::WindowSystem m_WindowSystem;
	Yuki::WindowHandle m_Window;
	Yuki::RHI::Swapchain m_Swapchain;
	Yuki::RHI::Queue m_GraphicsQueue;
	Yuki::RHI::Fence m_Fence;
	Yuki::RHI::CommandPool m_CommandPool;
	Yuki::RHI::Buffer m_VertexBuffer;
	Yuki::RHI::Buffer m_IndexBuffer;
	Yuki::RHI::RayTracingPipeline m_Pipeline;
	Yuki::RHI::DescriptorSetLayout m_DescriptorLayout;
	Yuki::RHI::DescriptorPool m_DescriptorPool;
	Yuki::RHI::DescriptorSet m_DescriptorSet;
	Yuki::RHI::AccelerationStructure m_AccelerationStructure;

	Yuki::RHI::Pipeline m_RasterizationPipeline;
	Yuki::RHI::DescriptorSetLayout m_RasterDescriptorLayout;
	Yuki::RHI::DescriptorPool m_RasterDescriptorPool;
	Yuki::RHI::DescriptorSet m_RasterDescriptorSet;

	Yuki::InputContext m_CameraInput;

	Yuki::Quat m_Rotation{1.0f, 0.0f, 0.0f, 0.0f};
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)