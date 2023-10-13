#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/Rendering/RenderGraph.hpp>
#include <Engine/Rendering/glTFLoader.hpp>

#include <Engine/Input/InputContext.hpp>

#include <iostream>

struct Vertex
{
	Yuki::Vec3 Position;
};

struct CameraData
{
	Yuki::Vec3 Position;
	Yuki::Quat Rotation;
};

static CameraData s_CameraData;

inline static bool s_RayTrace = true;

class EngineTesterApp : public Yuki::Application
{
public:
	EngineTesterApp()
		: Yuki::Application()
	{
		using namespace Yuki::RHI;

		m_Context = Context::Create({
			.RequestedFeatures = {
				RendererFeature::RayTracing,
			},
		});

		m_Window = m_WindowSystem.NewWindow({
			.Title = "My Window"
		});

		m_GraphicsQueue = m_Context.RequestQueue(QueueType::Graphics);

		m_Swapchain = Swapchain::Create(m_Context, m_WindowSystem, m_Window);

		m_Fence = Fence::Create(m_Context);
		m_CommandPool = CommandPool::Create(m_Context, m_GraphicsQueue);

		m_Graph = Yuki::Unique<Yuki::RenderGraph>::New(m_Context);

		struct RayTracingData
		{
			Image Output{};
			uint32_t Width = 0;
			uint32_t Height = 0;

			DescriptorSetLayout DescriptorLayout{};
			PipelineLayout PipelineLayout{};
			RayTracingPipeline Pipeline{};
			DescriptorPool DescriptorPool{};
			DescriptorSet DescriptorSet{};
			AccelerationStructure AccelerationStructure{};

			struct PushConstants
			{
				uint64_t TopLevelAS = 0;
				Yuki::Vec3 ViewPos = { 0.0f, 0.0f, 10.0f };
				Yuki::Vec3 CameraX = {};
				Yuki::Vec3 CameraY = {};
				float CameraZOffset = 0.0f;
			} PC;
		};

		m_Graph->AddPass({
			.Initialize = [&](auto& graph, int32_t passID)
			{
				using namespace Yuki::RHI;

				DescriptorCount dc = { DescriptorType::StorageImage, 1 };

				auto descriptorLayout = DescriptorSetLayout::Create(m_Context, {
					.Stages = ShaderStage::RayGeneration,
					.Descriptors = {
						{ 1, DescriptorType::StorageImage },
					}
				});

				auto pipelineLayout = PipelineLayout::Create(m_Context, {
					.PushConstantSize = sizeof(RayTracingData::PushConstants),
					.DescriptorLayouts = {
						descriptorLayout
					}
				});

				auto descriptorPool = DescriptorPool::Create(m_Context, { dc });

				RayTracingData data =
				{
					.DescriptorLayout = descriptorLayout,
					.PipelineLayout = pipelineLayout,
					.Pipeline = RayTracingPipeline::Create(m_Context, {
						.Layout = pipelineLayout,
						.Shaders = {
							{ "Shaders/Raytracing.glsl", ShaderStage::RayGeneration },
							{ "Shaders/Raytracing.glsl", ShaderStage::RayMiss },
							{ "Shaders/Raytracing.glsl", ShaderStage::RayClosestHit },
						}
					}),
					.DescriptorPool = descriptorPool,
					.DescriptorSet = descriptorPool.AllocateDescriptorSet(descriptorLayout),
					.AccelerationStructure = AccelerationStructure::Create(m_Context)
				};

				Yuki::glTFLoader loader;
				Yuki::Model model;
				loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes.gltf", model);

				Yuki::DynamicArray<GeometryID> geometries;

				for (const auto& mesh : model.Meshes)
					geometries.push_back(data.AccelerationStructure.AddGeometry(mesh.Positions, mesh.Indices));

				auto processNode = [&](this auto&& self, const Yuki::MeshNode& node, const Yuki::Mat4& parentTransform = Yuki::Mat4(1.0f)) -> void
				{
					Yuki::Mat4 transform = parentTransform * (glm::translate(node.Translation) * glm::mat4_cast(node.Rotation) * glm::scale(node.Scale));

					if (node.MeshIndex >= 0)
						data.AccelerationStructure.AddInstance(geometries[node.MeshIndex], transform);

					for (auto childNodeIndex : node.ChildNodes)
						self(model.Nodes[childNodeIndex], transform);
				};

				for (const auto& scene : model.Scenes)
				{
					for (auto nodeIndex : scene.NodeIndices)
						processNode(model.Nodes[nodeIndex]);
				}

				data.PC.CameraZOffset = 1.0f / glm::tan(0.5f * glm::radians(90.0f));

				graph.SetPassData(passID, data);
			},
			.Run = [&](auto& graph, int32_t passID)
			{
				if (!s_RayTrace)
					return;

				RayTracingData& data = graph.GetPassData<RayTracingData>(passID);
				data.PC.ViewPos = s_CameraData.Position;
				data.PC.CameraX = s_CameraData.Rotation * Yuki::Vec3{ 1.0f, 0.0f, 0.0f };
				data.PC.CameraY = s_CameraData.Rotation * Yuki::Vec3{ 0.0f, 1.0f, 0.0f };
				data.PC.TopLevelAS = data.AccelerationStructure.GetTopLevelAddress();

				const auto& windowData = m_WindowSystem.GetWindowData(m_Window);

				if (data.Width != windowData.Width || data.Height != windowData.Height)
				{
					if (data.Output)
						data.Output.Destroy();

					data.Output = Image::Create(m_Context, windowData.Width, windowData.Height, ImageFormat::BGRA8, ImageUsage::Storage | ImageUsage::TransferSource);
					data.Width = windowData.Width;
					data.Height = windowData.Height;
				}

				data.DescriptorSet.Write(0, { data.Output.GetDefaultView() }, 0);

				auto cmd = graph.StartPass();
				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::General } });
				cmd.PushConstants(data.PipelineLayout, Yuki::RHI::ShaderStage::RayGeneration, &data.PC, sizeof(data.PC));
				cmd.BindDescriptorSets(data.PipelineLayout, Yuki::RHI::PipelineBindPoint::RayTracing, { data.DescriptorSet });
				cmd.BindPipeline(data.Pipeline);
				cmd.TraceRays(data.Pipeline, data.Width, data.Height);
				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::TransferSource } });
				graph.EndPass(cmd);

				graph.Output(data.Output);
			}
		});

		struct RasterData
		{
			Image Output{};
			uint32_t Width = 0;
			uint32_t Height = 0;

			PipelineLayout PipelineLayout{};
			Pipeline Pipeline{};

			Buffer VertexBuffer;
			Buffer IndexBuffer;
			uint32_t IndexCount;

			struct PushConstants
			{
				uint64_t VertexBuffer;
				Yuki::Mat4 ViewProjection{};
			} PC;
		};

		m_Graph->AddPass({
			.Initialize = [&](auto& graph, int32_t passID)
			{
				using namespace Yuki::RHI;

				auto pipelineLayout = PipelineLayout::Create(m_Context, {
					.PushConstantSize = sizeof(RasterData::PushConstants),
					.DescriptorLayouts = {}
				});

				Yuki::glTFLoader loader;
				Yuki::Model model;
				loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes.gltf", model);

				RasterData data =
				{
					.PipelineLayout = pipelineLayout,
					.Pipeline = Pipeline::Create(m_Context, {
						.Layout = pipelineLayout,
						.Shaders = {
							{ "Shaders/Raster.glsl", ShaderStage::Vertex },
							{ "Shaders/Raster.glsl", ShaderStage::Fragment },
						},
						.ColorAttachments = {
							{ ImageFormat::BGRA8 }
						}
					}),
					.VertexBuffer = Buffer::Create(m_Context, model.Meshes[0].Positions.size() * sizeof(Yuki::Vec3), BufferUsage::Storage | BufferUsage::TransferDst),
					.IndexBuffer = Buffer::Create(m_Context, model.Meshes[0].Indices.size() * sizeof(uint32_t), BufferUsage::Index | BufferUsage::TransferDst),
					.IndexCount = Yuki::Cast<uint32_t>(model.Meshes[0].Indices.size())
				};

				Buffer::UploadImmediate(data.VertexBuffer, model.Meshes[0].Positions.data(), model.Meshes[0].Positions.size() * sizeof(Yuki::Vec3));
				Buffer::UploadImmediate(data.IndexBuffer, model.Meshes[0].Indices.data(), model.Meshes[0].Indices.size() * sizeof(uint32_t));

				graph.SetPassData(passID, data);
			},
			.Run = [&](auto& graph, int32_t passID)
			{
				if (s_RayTrace)
					return;

				RasterData& data = graph.GetPassData<RasterData>(passID);
				data.PC.VertexBuffer = data.VertexBuffer.GetDeviceAddress();

				Yuki::Mat4 projectionMatrix = Yuki::PerspectiveInfReversedZ(glm::radians(70.0f), Yuki::Cast<float>(data.Width) / Yuki::Cast<float>(data.Height), 0.001f);
				Yuki::Mat4 viewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), s_CameraData.Position) * glm::mat4_cast(s_CameraData.Rotation));
				data.PC.ViewProjection = projectionMatrix * viewMatrix;

				const auto& windowData = m_WindowSystem.GetWindowData(m_Window);

				if (data.Width != windowData.Width || data.Height != windowData.Height)
				{
					if (data.Output)
						data.Output.Destroy();

					data.Output = Image::Create(m_Context, windowData.Width, windowData.Height, ImageFormat::BGRA8, ImageUsage::ColorAttachment | ImageUsage::TransferSource);
					data.Width = windowData.Width;
					data.Height = windowData.Width;
				}

				auto cmd = graph.StartPass();
				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::Attachment } });
				cmd.PushConstants(data.PipelineLayout, Yuki::RHI::ShaderStage::RayGeneration, &data.PC, sizeof(data.PC));
				cmd.BindPipeline(data.Pipeline);
				cmd.SetViewport({
					.X = 0.0f,
					.Y = 0.0f,
					.Width = Yuki::Cast<float>(data.Width),
					.Height = Yuki::Cast<float>(data.Height),
				});
				cmd.BeginRendering({
					.ColorAttachments = {
						{
							.ImageView = data.Output.GetDefaultView(),
							.LoadOp = Yuki::RHI::AttachmentLoadOp::Clear,
							.StoreOp = Yuki::RHI::AttachmentStoreOp::Store
						}
					}
				});

				cmd.BindIndexBuffer(data.IndexBuffer);
				cmd.DrawIndexed(data.IndexCount, 0, 0);

				cmd.EndRendering();

				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::TransferSource } });

				graph.EndPass(cmd);

				graph.Output(data.Output);
			}
		});

		m_Graph->AddPass({
			.Initialize = [](const auto& graph, int32_t passID){},
			.Run = [&](auto& graph, int32_t passID)
			{
				const auto& image = graph.Input<Yuki::RHI::Image>();
				auto swapchainImage = m_Swapchain.GetCurrentImage();

				auto cmd = graph.StartPass();
				cmd.ImageBarrier({ { swapchainImage }, { Yuki::RHI::ImageLayout::TransferDest } });
				cmd.BlitImage(swapchainImage, image);
				cmd.ImageBarrier({ { swapchainImage }, { Yuki::RHI::ImageLayout::Present } });
				graph.EndPass(cmd);
			}
		});
	}

	void Update() override
	{
		m_WindowSystem.PollMessages();

		UpdateCamera();

		m_Fence.Wait();
		m_GraphicsQueue.AcquireImages({ m_Swapchain }, { m_Fence });
		m_Graph->Execute();
		m_GraphicsQueue.Present({ m_Swapchain }, { m_Fence });
	}

private:
	void UpdateCamera()
	{
		Yuki::Vec3 translation = {};

		if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::W))
		{
			translation.z -= 5.0f;
		}
		else if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::S))
		{
			translation.z += 5.0f;
		}

		if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::A))
		{
			translation.x -= 5.0f;
		}
		else if (m_WindowSystem.GetKeyState(m_Window, Yuki::KeyCode::D))
		{
			translation.x += 5.0f;
		}

		if (m_WindowSystem.GetMouseButtonState(m_Window, Yuki::MouseButton::Right))
		{
			m_WindowSystem.SetCursorLock(m_Window, true);

			int64_t deltaX = m_WindowSystem.GetRawMouseDeltaX(m_Window) * -1;
			int64_t deltaY = m_WindowSystem.GetRawMouseDeltaY(m_Window);

			m_CameraRotation = glm::angleAxis(deltaX * 0.001f, Yuki::Vec3{ 0.0f, 1.0f, 0.0f }) * m_CameraRotation;
			auto pitchedRotation = m_CameraRotation * glm::angleAxis(deltaY * 0.001f, Yuki::Vec3{ -1.0f, 0.0f, 0.0f });
			if (glm::dot(pitchedRotation * Yuki::Vec3{ 0.0f, 1.0f, 0.0f }, Yuki::Vec3{ 0.0f, 1.0f, 0.0f }) >= 0.0f)
			{
				m_CameraRotation = pitchedRotation;
			}
			m_CameraRotation = glm::normalize(m_CameraRotation);
		}
		else
		{
			m_WindowSystem.SetCursorLock(m_Window, false);
		}

		m_CameraPosition += (m_CameraRotation * translation) * Yuki::EngineTime::DeltaTime<float>();
		s_CameraData = { m_CameraPosition, m_CameraRotation };
	}

private:
	Yuki::RHI::Context m_Context;
	Yuki::WindowSystem m_WindowSystem;
	Yuki::WindowHandle m_Window;
	Yuki::Unique<Yuki::RenderGraph> m_Graph;
	Yuki::RHI::Swapchain m_Swapchain;
	Yuki::RHI::Queue m_GraphicsQueue;
	Yuki::RHI::Fence m_Fence;
	Yuki::RHI::CommandPool m_CommandPool;

	Yuki::InputContext m_CameraInput;

	Yuki::Vec3 m_CameraPosition{ 0.0f, 0.0f, 5.0f };
	Yuki::Quat m_CameraRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)