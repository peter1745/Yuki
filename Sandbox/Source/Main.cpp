#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp>
#include <Yuki/Rendering/RHI/ShaderCompiler.hpp>
#include <Yuki/Rendering/RHI/RenderTarget.hpp>
#include <Yuki/Rendering/RHI/Queue.hpp>
#include <Yuki/Rendering/RHI/Fence.hpp>
#include <Yuki/Rendering/SceneRenderer.hpp>
#include <Yuki/IO/MeshLoader.hpp>

class TestApplication : public Yuki::Application
{
public:
	TestApplication()
	    : Yuki::Application("Test Application")
	{
	}

private:
	void CreateWindow(Yuki::WindowAttributes InWindowAttributes)
	{
		auto* window = NewWindow(std::move(InWindowAttributes));
		window->Show();
		m_Windows.emplace_back(window);
	}

	void OnInitialize() override
	{
		CreateWindow({
			.Title = "My Main Window",
			.Width = 1920,
			.Height = 1080
		});

		m_Fence = GetRenderContext()->CreateFence();

		m_Renderer = new Yuki::SceneRenderer(GetRenderContext(), m_Windows[0]->GetViewport());

		//m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/NewSponza_Main_glTF_002.gltf");

		m_CameraTransform.SetIdentity();
	}

	void OnRunLoop() override
	{
		UpdateInput();

		// Collect Viewports
		std::vector<Yuki::Viewport*> viewports;
		viewports.reserve(m_Windows.size());
		for (auto* window : m_Windows)
		{
			if (window == nullptr || window->GetViewport() == nullptr)
				continue;

			viewports.emplace_back(window->GetViewport());
		}

		m_Fence->Wait();

		// Acquire Images for all Viewports
		GetRenderContext()->GetGraphicsQueue()->AcquireImages(viewports, { m_Fence });

		m_Renderer->BeginDraw(m_CameraTransform);
		m_Renderer->DrawMesh(m_Mesh);
		m_Renderer->EndDraw();

		GetRenderContext()->GetGraphicsQueue()->SubmitCommandBuffers({ m_Renderer->GetCurrentCommandBuffer() }, { m_Fence }, {});

		// Present all swapchain images
		GetRenderContext()->GetGraphicsQueue()->Present(viewports, { m_Fence });
	}

	void UpdateInput()
	{
		const float movementSpeed = 0.01f;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::W))
			m_CameraTranslation.Z -= movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::S))
			m_CameraTranslation.Z += movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::D))
			m_CameraTranslation.X -= movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::A))
			m_CameraTranslation.X += movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::LeftShift))
			m_CameraTranslation.Y -= movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::Space))
			m_CameraTranslation.Y += movementSpeed;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::Q))
			m_CameraRotation += 1.0f;

		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::E))
			m_CameraRotation -= 1.0f;

		m_CameraTransform = Yuki::Math::Mat4::Translation(m_CameraTranslation);
		m_CameraTransform *= Yuki::Math::Mat4::Rotation(Yuki::Math::Quat(Yuki::Math::Radians(m_CameraRotation), { 0.0f, 1.0f, 0.0f }));
	}

	void OnDestroy() override
	{
		GetRenderContext()->DestroyFence(m_Fence);
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Fence* m_Fence = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::LoadedMesh m_Mesh;

	Yuki::Math::Mat4 m_CameraTransform;
	Yuki::Math::Vec3 m_CameraTranslation{0.0f, 0.0f, 0.0f};
	float m_CameraRotation = 0.0f;
};

YUKI_DECLARE_APPLICATION(TestApplication)
