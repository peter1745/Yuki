#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/Containers/IndexFreeList.hpp>

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

		m_DescriptorHeap = DescriptorHeap::Create(m_Context, 65536);
		m_Fence = Fence::Create(m_Context);

		m_Graph = Yuki::Unique<Yuki::RenderGraph>::New(m_Context);

		struct GeometryInfo
		{
			uint64_t ShadingAttribs;
			uint64_t Indices;
		};

		struct RayTracingData
		{
			Image Output{};
			uint32_t Width = 0;
			uint32_t Height = 0;

			PipelineLayout PipelineLayout{};
			RayTracingPipeline Pipeline{};
			AccelerationStructure AccelerationStructure{};

			Buffer GeometriesBuffer;
			Yuki::DynamicArray<Buffer> IndexBuffers;
			Yuki::DynamicArray<Buffer> ShadingAttributeBuffers;
			Buffer MaterialBuffer;

			struct PushConstants
			{
				uint64_t TopLevelAS = 0;
				Yuki::Vec3 ViewPos = { 0.0f, 0.0f, 10.0f };
				Yuki::Vec3 CameraX = {};
				Yuki::Vec3 CameraY = {};
				float CameraZOffset = 0.0f;
				uint64_t Geometries = 0;
				uint64_t Materials = 0;
				uint32_t OutputImageHandle;
				uint32_t DefaultSamplerHandle;
			} PC;
		};

		m_Graph->AddPass({
			.Initialize = [&](auto& graph, int32_t passID)
			{
				using namespace Yuki::RHI;

				Yuki::glTFLoader loader;
				Yuki::Model model;
				//loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes.gltf", model);
				loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes_Textured_Complex.gltf", model);
				//loader.Load("Meshes/NewSponza_Main_glTF_002.gltf", model);

				auto pipelineLayout = PipelineLayout::Create(m_Context, {
					.PushConstantSize = sizeof(RayTracingData::PushConstants),
				});

				RayTracingData data =
				{
					.PipelineLayout = pipelineLayout,
					.Pipeline = RayTracingPipeline::Create(m_Context, {
						.Layout = pipelineLayout,
						.Shaders = {
							{ "Shaders/RayGen.glsl", ShaderStage::RayGeneration },
							{ "Shaders/RayMiss.glsl", ShaderStage::RayMiss },
							{ "Shaders/RayClosestHit.glsl", ShaderStage::RayClosestHit },
						}
					}),
					.AccelerationStructure = AccelerationStructure::Create(m_Context)
				};

				for (size_t i = 0; i < model.Textures.size(); i++)
				{
					uint32_t handle = m_HeapFreeList.Acquire();

					for (auto& material : model.Materials)
					{
						if (material.BaseColorTextureIndex == i)
							material.BaseColorTextureIndex = handle;
					}

					const auto& texture = model.Textures[i];
					Image image = Image::Create(m_Context, texture.Width, texture.Height, ImageFormat::RGBA8, ImageUsage::Sampled | ImageUsage::TransferDest | ImageUsage::HostTransfer);
					image.SetData(texture.Data.data());
					m_DescriptorHeap.WriteSampledImages(handle, { image.GetDefaultView() });
				}

				data.PC.DefaultSamplerHandle = m_HeapFreeList.Acquire();
				data.PC.OutputImageHandle = m_HeapFreeList.Acquire();

				auto sampler = Sampler::Create(m_Context);
				m_DescriptorHeap.WriteSamplers(data.PC.DefaultSamplerHandle, { sampler });

				data.GeometriesBuffer = Buffer::Create(m_Context, 65536 * sizeof(GeometryInfo), BufferUsage::Storage | BufferUsage::TransferDst, BufferFlags::Mapped | BufferFlags::DeviceLocal);
				data.MaterialBuffer = Buffer::Create(m_Context, model.Materials.size() * sizeof(Yuki::MeshMaterial), BufferUsage::Storage | BufferUsage::TransferDst, BufferFlags::Mapped | BufferFlags::DeviceLocal);
				data.MaterialBuffer.Set<Yuki::MeshMaterial>(model.Materials);

				data.PC.Geometries = data.GeometriesBuffer.GetDeviceAddress();
				data.PC.Materials = data.MaterialBuffer.GetDeviceAddress();

				Yuki::DynamicArray<GeometryID> geometries;

				for (const auto& mesh : model.Meshes)
				{
					Buffer indexBuffer = Buffer::Create(m_Context, mesh.Indices.size() * sizeof(uint32_t),
														BufferUsage::Storage |
														BufferUsage::TransferDst,
														BufferFlags::Mapped |
														BufferFlags::DeviceLocal);
					Buffer shadingAttribsBuffer = Buffer::Create(m_Context, mesh.ShadingAttributes.size() * sizeof(Yuki::ShadingAttributes),
																BufferUsage::Storage |
																BufferUsage::TransferDst,
																BufferFlags::Mapped |
																BufferFlags::DeviceLocal);

					indexBuffer.Set<uint32_t>(mesh.Indices);
					shadingAttribsBuffer.Set<Yuki::ShadingAttributes>(mesh.ShadingAttributes);

					data.IndexBuffers.push_back(indexBuffer);
					data.ShadingAttributeBuffers.push_back(shadingAttribsBuffer);

					GeometryInfo geometryInfo =
					{
						.ShadingAttribs = shadingAttribsBuffer.GetDeviceAddress(),
						.Indices = indexBuffer.GetDeviceAddress()
					};
					data.GeometriesBuffer.Set(geometryInfo, Yuki::Cast<uint32_t>(geometries.size()));

					geometries.push_back(data.AccelerationStructure.AddGeometry(mesh.Positions, mesh.Indices));
				}

				auto processNode = [&](this auto&& self, const Yuki::MeshNode& node, const Yuki::Mat4& parentTransform = Yuki::Mat4(1.0f)) -> void
				{
					Yuki::Mat4 transform = parentTransform * (glm::translate(node.Translation) * glm::mat4_cast(node.Rotation) * glm::scale(node.Scale));

					if (node.MeshIndex >= 0)
						data.AccelerationStructure.AddInstance(geometries[node.MeshIndex], transform, node.MeshIndex);

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
				
					m_DescriptorHeap.WriteStorageImages(data.PC.OutputImageHandle, { data.Output.GetDefaultView() });
				}

				auto cmd = graph.StartPass();
				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::General } });
				cmd.PushConstants(data.PipelineLayout, Yuki::RHI::ShaderStage::RayGeneration, &data.PC, sizeof(data.PC));
				cmd.BindDescriptorHeap(data.PipelineLayout, Yuki::RHI::PipelineBindPoint::RayTracing, m_DescriptorHeap);
				cmd.BindPipeline(data.Pipeline);
				cmd.TraceRays(data.Pipeline, data.Width, data.Height);
				cmd.ImageBarrier({ { data.Output }, { Yuki::RHI::ImageLayout::TransferSource } });
				graph.EndPass(cmd);

				graph.Output(data.Output);
			}
		});

		/*struct RasterData
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
					data.Height = windowData.Height;
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
		});*/

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
	Yuki::RHI::DescriptorHeap m_DescriptorHeap;
	Yuki::RHI::Fence m_Fence;

	Yuki::InputContext m_CameraInput;

	Yuki::Vec3 m_CameraPosition{ 0.0f, 0.0f, 5.0f };
	Yuki::Quat m_CameraRotation{ 1.0f, 0.0f, 0.0f, 0.0f };

	Yuki::IndexFreeList m_HeapFreeList;
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)