#include <Engine/Common/EntryPoint.hpp>
#include <Engine/Common/EngineTime.hpp>
#include <Engine/Common/WindowSystem.hpp>

#include <Engine/Containers/IndexFreeList.hpp>

#include <Engine/Rendering/RenderGraph.hpp>
#include <Engine/Rendering/RTRenderer.hpp>
#include <Engine/Rendering/glTFLoader.hpp>

#include <Engine/Input/InputContext.hpp>

#include <iostream>

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

		m_Renderer = Yuki::Unique<Yuki::RTRenderer>::New(m_Context);

		Yuki::glTFLoader loader;
		Yuki::Model model;
		//loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes.gltf", model);
		//loader.Load("Meshes/deccer-cubes-main/SM_Deccer_Cubes_Textured_Complex.gltf", model);
		loader.Load("Meshes/NewSponza_Main_glTF_002.gltf", model);
		m_Renderer->AddMesh(model);
	}

	void Update() override
	{
		m_WindowSystem.PollMessages();

		UpdateCamera();

		m_Fence.Wait();

		m_GraphicsQueue.AcquireImages({ m_Swapchain }, { m_Fence });
		m_Renderer->Render(m_Swapchain.GetCurrentImage(), m_Fence, { m_CameraPosition, m_CameraRotation });
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
	}

private:
	Yuki::RHI::Context m_Context;
	Yuki::WindowSystem m_WindowSystem;
	Yuki::WindowHandle m_Window;
	Yuki::RHI::Swapchain m_Swapchain;
	Yuki::RHI::Queue m_GraphicsQueue;
	Yuki::RHI::DescriptorHeap m_DescriptorHeap;
	Yuki::RHI::Fence m_Fence;

	Yuki::Unique<Yuki::RTRenderer> m_Renderer;

	Yuki::InputContext m_CameraInput;

	Yuki::Vec3 m_CameraPosition{ 0.0f, 0.0f, 5.0f };
	Yuki::Quat m_CameraRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
};

YUKI_DECLARE_APPLICATION(EngineTesterApp)