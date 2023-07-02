#include "FreeCamera.hpp"

#include <iostream>
#include <filesystem>
#include <array>
#include <span>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/Core/Timer.hpp>
#include <Yuki/Core/ResourceRegistry.hpp>
#include <Yuki/Core/ScopeExitGuard.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/Math/Mat4.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/PipelineBuilder.hpp>
#include <Yuki/Rendering/DescriptorSetBuilder.hpp>
#include <Yuki/Rendering/SceneRenderer.hpp>
#include <Yuki/Rendering/EntityRenderer.hpp>
#include <Yuki/IO/MeshLoader.hpp>

#include <Yuki/Entities/TransformComponents.hpp>
#include <Yuki/Entities/RenderingComponents.hpp>

#include <Yuki/ImGui/ImGuiWindowContext.hpp>
#include <Yuki/ImGui/ImGuiRenderContext.hpp>

#include <nfd.hpp>

#include <imgui/imgui.h>
#include <flecs/flecs.h>

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

		m_GraphicsQueue = { GetRenderContext()->GetGraphicsQueue(), GetRenderContext() };

		m_Fence = Yuki::Fence(GetRenderContext());

		m_Renderer = new Yuki::EntityRenderer(GetRenderContext(), m_World);

		m_MeshLoader = Yuki::Unique<Yuki::MeshLoader>::Create(GetRenderContext(), [this](Yuki::Mesh InMesh)
		{
			auto handle = m_Renderer->SubmitForUpload(std::move(InMesh));
			std::scoped_lock lock(m_Mutex);
			m_LoadedMeshes.push_back(handle);
		});

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);

		m_CameraEntity = CreateEntity("Camera Entity").add<Yuki::Entities::CameraComponent>();

		NFD::Init();

		InitializeImGui();
	}

	void InitializeImGui()
	{
		ImGui::SetCurrentContext(ImGui::CreateContext());
		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImGui::StyleColorsDark();

		m_ImGuiWindowContext = Yuki::Unique<Yuki::ImGuiWindowContext>::Create(m_Windows[0]);
		m_ImGuiRenderContext = Yuki::ImGuiRenderContext::New(m_Windows[0]->GetSwapchain(), GetRenderContext());

		m_CommandPool = Yuki::CommandPool(GetRenderContext(), m_GraphicsQueue);
		m_ImGuiCommandList = m_CommandPool.CreateCommandList();

		auto& style = ImGui::GetStyle();
		style.FramePadding = ImVec2(6.0f, 6.0f);

		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	void OnRunLoop(float InDeltaTime) override
	{
		m_Camera->Update(InDeltaTime);

		m_World.progress();

		const auto& windowAttribs = m_Windows[0]->GetAttributes();
		if (windowAttribs.Width == 0 || windowAttribs.Height == 0)
			return;

		m_Fence.Wait();
		RenderWorld();
		RenderImGui();
	}

	void RenderWorld()
	{
		m_Renderer->Reset();

		GetRenderContext()->GetTransferScheduler().Execute();

		if (m_ViewportWidth != m_LastViewportWidth || m_ViewportHeight != m_LastViewportHeight)
		{
			m_Renderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_ImGuiRenderContext->RecreateImage(m_Renderer->GetFinalImage());
		}

		m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, (float)m_ViewportWidth / m_ViewportHeight, 0.05f) * m_Camera->GetViewMatrix());
		m_Renderer->RenderEntities();
		m_Renderer->EndFrame();
	}

	void RenderImGui()
	{
		auto swapchains = GetRenderContext()->GetSwapchains();

		if (swapchains.empty())
			return;

		// Acquire Images for all Viewports
		m_GraphicsQueue.AcquireImages(swapchains, { m_Fence });

		m_CommandPool.Reset();

		m_ImGuiCommandList.Begin();

		m_ImGuiRenderContext->NewFrame(m_ImGuiCommandList);
		m_ImGuiWindowContext->NewFrame();
		ImGui::NewFrame();

		DrawUI();

		ImGui::Render();
		m_ImGuiRenderContext->EndFrame(m_ImGuiCommandList);
		m_ImGuiCommandList.PrepareSwapchainPresent(m_Windows[0]->GetSwapchain());
		m_ImGuiCommandList.End();

		m_GraphicsQueue.SubmitCommandLists({ m_ImGuiCommandList }, { m_Fence }, { m_Fence });

		// Present all swapchain images
		m_GraphicsQueue.Present(swapchains, { m_Fence });
	}

	flecs::entity CreateEntity(std::string_view InName)
	{
		auto entity = m_World.entity(InName.data())
			.set([](Yuki::Entities::Translation& InTranslation, Yuki::Entities::Rotation& InRotation, Yuki::Entities::Scale& InScale)
			{
				InTranslation.Value = { 0.0f, 0.0f, 0.0f };
				InRotation.Value.Value = { 0.0f, 0.0f, 0.0f, 1.0f };
				InScale.Value = 1.0f;
			});

		m_Entities.push_back(entity);
		m_EntityExpandState[entity] = false;
		return entity;
	}

	void BeginMainDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(mainViewport->Pos);
		ImGui::SetNextWindowSize(mainViewport->Size);
		ImGui::SetNextWindowViewport(mainViewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_MenuBar;

		ImGui::Begin("MainDockspaceWindow", nullptr, flags);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Import..."))
				{
					NFD::UniquePathSet filePaths;
					nfdresult_t result = NFD::OpenDialogMultiple(filePaths, static_cast<const nfdnfilteritem_t*>(nullptr));

					if (result == NFD_OKAY)
					{
						nfdpathsetsize_t pathCount;
        				NFD::PathSet::Count(filePaths, pathCount);

						for (uint32_t i = 0; i < pathCount; i++)
						{
							NFD::UniquePathSetPath path;
            				NFD::PathSet::GetPath(filePaths, i, path);
							m_MeshLoader->LoadGLTFMesh(path.get());
						}
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::DockSpace(ImGui::GetID("MainDockspace"));

		ImGui::PopStyleVar();
	}

	void EndMainDockspace()
	{
		ImGui::End();
	}

	void DrawUI()
	{
		BeginMainDockspace();

		if (ImGui::Begin("Settings"))
		{
			ImGui::DragFloat("Camera Speed", &m_Camera->GetMovementSpeed());
		}
		ImGui::End();

		DrawViewport();
		DrawEntityList();
		DrawEntityData();
		DrawContentBrowser();

		EndMainDockspace();
	}

	void DrawContentBrowser()
	{
		YUKI_SCOPE_EXIT_GUARD(){ ImGui::End(); };

		if (!ImGui::Begin("Content Browser"))
			return;

		for (auto meshHandle : m_LoadedMeshes)
		{
			const auto& mesh = m_Renderer->GetMesh(meshHandle);
			auto name = mesh.FilePath.filename().string();
			auto label = fmt::format("Create {}", name);
			if (ImGui::Button(label.c_str()))
			{
				CreateEntity(name)
					.set([meshHandle](Yuki::Entities::MeshComponent& InMeshComp)
					{
						InMeshComp.Value = meshHandle;
					});
			}
		}
	}

	void DrawViewport()
	{
		ImVec2 viewportSize{0.0f, 0.0f};
		m_LastViewportWidth = m_ViewportWidth;
		m_LastViewportHeight = m_ViewportHeight;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("Viewport"))
		{
			viewportSize = ImGui::GetContentRegionAvail();
			m_ImGuiRenderContext->DrawImage(m_Renderer->GetFinalImage(), viewportSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if (uint32_t(viewportSize.x) > 0 && uint32_t(viewportSize.y) > 0)
		{
			m_ViewportWidth = uint32_t(viewportSize.x);
			m_ViewportHeight = uint32_t(viewportSize.y);
		}
	}

	void DrawEntityData()
	{
		YUKI_SCOPE_EXIT_GUARD(){ ImGui::End(); };

		if (!ImGui::Begin("Entity Data"))
			return;

		if (m_SelectedEntity == flecs::entity::null())
			return;

		ImGui::Text("Name: %s", m_SelectedEntity.name().c_str());

		auto* translation = m_SelectedEntity.get_mut<Yuki::Entities::Translation>();
		auto* rotation = m_SelectedEntity.get_mut<Yuki::Entities::Rotation>();
		auto* scale = m_SelectedEntity.get_mut<Yuki::Entities::Scale>();

		bool changed = false;

		auto& eulerAngles = m_EulerCache[m_SelectedEntity];

		changed |= ImGui::DragFloat3("Translation", &translation->Value[0], 0.5f);
		
		if (ImGui::DragFloat3("Rotation", &eulerAngles[0], 0.5f))
		{
			Yuki::Math::Vec3 radians;
			radians.X = Yuki::Math::Radians(eulerAngles.X);
			radians.Y = Yuki::Math::Radians(eulerAngles.Y);
			radians.Z = Yuki::Math::Radians(eulerAngles.Z);
			rotation->Value = Yuki::Math::Quat(radians);

			changed = true;
		}

		changed |= ImGui::DragFloat("Scale", &scale->Value, 0.5f);

		if (changed)
		{
			m_Renderer->SynchronizeGPUTransform(m_SelectedEntity);
		}
	}

	void DrawEntityListItem(flecs::entity InEntity)
	{
		auto label = fmt::format("{}", InEntity.name().c_str());
		ImGui::ArrowButton("##ArrowButton", m_EntityExpandState[InEntity] ? ImGuiDir_Down : ImGuiDir_Right);

		if (ImGui::IsItemClicked())
			m_EntityExpandState[InEntity] = !m_EntityExpandState[InEntity];

		ImGui::SameLine();
		ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap);

		if (ImGui::IsItemClicked())
			m_SelectedEntity = InEntity;
	}

	void DrawEntityList()
	{
		YUKI_SCOPE_EXIT_GUARD(){ ImGui::End(); };
		if (!ImGui::Begin("Entities"))
			return;

		if (ImGui::Button("New Entity"))
		{
			auto entity = CreateEntity("New Entity");
			YUKI_UNUSED(entity);
		}

		ImGui::Separator();

		ImGuiListClipper clipper;
		clipper.Begin(int32_t(m_Entities.size()));

		while (clipper.Step())
		{
			for (int32_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				DrawEntityListItem(m_Entities[size_t(i)]);
		}
	}

	void OnDestroy() override
	{
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Queue m_GraphicsQueue{};
	Yuki::Fence m_Fence{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;
	Yuki::Unique<Yuki::MeshLoader> m_MeshLoader = nullptr;

	Yuki::EntityRenderer* m_Renderer;

	Yuki::Unique<Yuki::ImGuiWindowContext> m_ImGuiWindowContext;
	Yuki::Unique<Yuki::ImGuiRenderContext> m_ImGuiRenderContext;

	Yuki::CommandPool m_CommandPool{};
	Yuki::CommandList m_ImGuiCommandList{};

	std::shared_mutex m_Mutex;
	Yuki::DynamicArray<Yuki::MeshHandle> m_LoadedMeshes;

	flecs::world m_World;
	flecs::entity m_CameraEntity;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	uint32_t m_LastViewportWidth = 0;
	uint32_t m_LastViewportHeight = 0;

	Yuki::DynamicArray<flecs::entity> m_Entities;
	Yuki::Map<flecs::entity, bool, Yuki::FlecsEntityHash> m_EntityExpandState;

	flecs::entity m_SelectedEntity = flecs::entity::null();

	Yuki::Map<flecs::entity, Yuki::Math::Vec3, Yuki::FlecsEntityHash> m_EulerCache;
};

YUKI_DECLARE_APPLICATION(TestApplication)
