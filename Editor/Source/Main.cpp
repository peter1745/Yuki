#include "FreeCamera.hpp"
#include "EditorPanels/EditorPanel.hpp"
#include "EditorPanels/ContentBrowser.hpp"
#include "EditorPanels/EditorViewport.hpp"
#include "EditorPanels/EntityDetails.hpp"

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
#include <Yuki/Rendering/EntityRenderer.hpp>
#include <Yuki/Rendering/MeshGenerator.hpp>

#include <Yuki/World/World.hpp>
#include <Yuki/World/Components/CoreComponents.hpp>
#include <Yuki/World/Components/TransformComponents.hpp>
#include <Yuki/World/Components/RenderingComponents.hpp>
#include <Yuki/World/Components/SpaceComponents.hpp>

#include <Yuki/ImGui/ImGuiWindowContext.hpp>
#include <Yuki/ImGui/ImGuiRenderContext.hpp>

#include <nfd.hpp>

#include <imgui/imgui.h>

#include <FastNoise/FastNoise.h>

namespace YukiEditor {

	struct HierarchyNodeID
	{
		flecs::id Parent;
		flecs::id Child;

		bool operator==(const HierarchyNodeID& InOther) const { return (Parent == InOther.Parent && Child == InOther.Child); }
	};

	struct HierarchyNodeIDHash
	{
		using is_avalanching = void;

		uint64_t operator()(const HierarchyNodeID& InID) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InID, sizeof(HierarchyNodeID));
		}
	};

	struct SceneHierarchyNode
	{
		HierarchyNodeID ID;
		bool Expanded = false;
		bool Expandable = true;
		int32_t ParentIndex = -1;
	};

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
			auto* window = NewWindow(m_RenderContext, std::move(InWindowAttributes));
			window->Show();
			m_Windows.emplace_back(window);
		}

		void OnInitialize() override
		{
			m_RenderContext = Yuki::RenderContext::New(Yuki::RenderAPI::Vulkan);

			CreateWindow({
				.Title = "My Main Window",
				.Width = 1920,
				.Height = 1080
			});

			m_GraphicsQueue = { m_RenderContext->GetGraphicsQueue(), m_RenderContext };

			m_Fence = Yuki::Fence(m_RenderContext);

			m_AssetRegistry = Yuki::Unique<Yuki::AssetRegistry>::Create("Content/AssetRegistry.json");
			m_AssetSystem = Yuki::Unique<Yuki::AssetSystem>::Create(*m_AssetRegistry);

			m_World.SetOnEntityCreateCallback([this](flecs::entity InEntity)
			{
				m_RebuildSceneHierarchy = true;
			});

			NFD::Init();

			InitializeImGui();
			InitializeEditorUI();
		}

		void InitializeEditorUI()
		{
			m_EditorPanels.emplace_back(std::make_unique<ContentBrowser>(*m_AssetRegistry));

			auto viewport = std::make_unique<EditorViewport>(*m_AssetSystem, m_Windows[0], m_RenderContext, m_ImGuiRenderContext, &m_World);
			m_EditorPanels.emplace_back(std::move(viewport));
			m_EditorPanels.emplace_back(std::make_unique<EntityDetails>());
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
			m_ImGuiRenderContext = Yuki::ImGuiRenderContext::New(m_Windows[0]->GetSwapchain(), m_RenderContext);

			m_CommandPool = Yuki::CommandPool(m_RenderContext, m_GraphicsQueue);
			m_ImGuiCommandList = m_CommandPool.CreateCommandList();

			auto& style = ImGui::GetStyle();
			style.FramePadding = ImVec2(6.0f, 6.0f);

			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		void OnRunLoop(float InDeltaTime) override
		{
			m_Fence.Wait();

			m_World.Tick(InDeltaTime);

			const auto& windowAttribs = m_Windows[0]->GetAttributes();
			if (windowAttribs.Width == 0 || windowAttribs.Height == 0)
				return;

			for (auto& panel : m_EditorPanels)
				panel->Update(InDeltaTime);

			RenderImGui();
		}

		void RenderImGui()
		{
			auto swapchains = m_RenderContext->GetSwapchains();

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
				YUKI_SCOPE_EXIT_GUARD() { ImGui::EndMainMenuBar(); };

				if (ImGui::BeginMenu("File"))
				{
					YUKI_SCOPE_EXIT_GUARD() { ImGui::EndMenu(); };
				}
			}

			ImGui::DockSpace(ImGui::GetID("MainDockspace"));

			ImGui::PopStyleVar();
		}

		void DrawUI()
		{
			YUKI_SCOPE_EXIT_GUARD() { ImGui::End(); };

			BeginMainDockspace();

			static bool s_Wireframe = false;

			if (ImGui::Begin("Settings"))
			{
				ImGui::Text("Delta Time: %.4f", GetDeltaTime());

				if (ImGui::Checkbox("Wireframe", &s_Wireframe))
				{
					static_cast<EditorViewport*>(m_EditorPanels[1].get())->GetRenderer()->SetWireframe(s_Wireframe);
				}

				//ImGui::DragFloat("Camera Speed", &m_Camera->GetMovementSpeed());

				if (ImGui::Button("Start"))
					m_World.StartSimulation();

				if (ImGui::Button("Cube Sphere"))
				{
					auto assetID = Yuki::MeshGenerator::GenerateCubeSphere(*m_AssetSystem, 10.0f, 32, 10.0f);
					auto* asset = m_AssetSystem->Request<Yuki::MeshAsset>(assetID);

					auto* texture = new Yuki::TextureAsset();
					texture->Width = 1024;
					texture->Height = 1024;
					auto noiseGen = FastNoise::New<FastNoise::CellularDistance>();
					auto rgba8Convert = FastNoise::New<FastNoise::ConvertRGBA8>();
					rgba8Convert->SetSource(noiseGen);
					texture->Data = new std::byte[texture->Width * texture->Height * 4];
					rgba8Convert->GenTileable2D(reinterpret_cast<float*>(texture->Data), texture->Width, texture->Height, 0.01f, static_cast<int32_t>(time(NULL)));
					asset->Scene.Textures.push_back(m_AssetSystem->AddAsset(Yuki::AssetType::Texture, texture));

					flecs::entity entity = m_World.InstantiateMeshScene(assetID, asset->Scene);
					static_cast<EditorViewport*>(m_EditorPanels[1].get())->GetRenderer()->SubmitForUpload(assetID, *m_AssetSystem, asset->Scene);
					static_cast<EditorViewport*>(m_EditorPanels[1].get())->GetRenderer()->CreateGPUInstance(entity);
				}

				if (ImGui::Button("Icosphere"))
				{
					auto assetID = Yuki::MeshGenerator::GenerateIcosphere(*m_AssetSystem, 32, 4.0f);
					auto* asset = m_AssetSystem->Request<Yuki::MeshAsset>(assetID);

					// TODO(Peter): Create a StarComponent or some sort of start generation system

					auto* texture = new Yuki::TextureAsset();
					texture->Width = 512;
					texture->Height = 512;
					auto noiseGen = FastNoise::New<FastNoise::CellularDistance>();
					auto rgba8Convert = FastNoise::New<FastNoise::ConvertRGBA8>();
					rgba8Convert->SetSource(noiseGen);
					texture->Data = new std::byte[texture->Width * texture->Height * 4];
					rgba8Convert->GenTileable2D(reinterpret_cast<float*>(texture->Data), texture->Width, texture->Height, 0.01f, static_cast<int32_t>(time(NULL)));
					asset->Scene.Textures.push_back(m_AssetSystem->AddAsset(Yuki::AssetType::Texture, texture));

					flecs::entity entity = m_World.InstantiateMeshScene(assetID, asset->Scene);
					entity.add<Yuki::Components::StarGenerator>();

					static_cast<EditorViewport*>(m_EditorPanels[1].get())->GetRenderer()->SubmitForUpload(assetID, *m_AssetSystem, asset->Scene);
					static_cast<EditorViewport*>(m_EditorPanels[1].get())->GetRenderer()->CreateGPUInstance(entity);
				}
			}
			ImGui::End();

			DrawEntityList();

			for (auto& panel : m_EditorPanels)
				panel->Draw();
		}

		void AddToHierarchy(Yuki::DynamicArray<SceneHierarchyNode>& InHierarchy, flecs::id InID, flecs::id InParentID, bool InExpandable, int32_t InParentIndex = -1)
		{
			HierarchyNodeID id{ InParentID, InID };
			if (m_SceneHierarchyCacheIndexLookup.contains(id))
			{
				const auto& oldNode = m_SceneHierarchyCache[m_SceneHierarchyCacheIndexLookup[id]];
				auto& newNode = InHierarchy.emplace_back();
				newNode.ID = id;
				newNode.Expanded = oldNode.Expanded;
				newNode.Expandable = oldNode.Expandable;
				newNode.ParentIndex = InParentIndex;

				if (newNode.Expanded && InID.is_entity())
				{
					InID.entity().each([this, &InID, &InHierarchy, nodeIndex = InHierarchy.size() - 1](flecs::id InChild)
					{
						if (InChild.is_pair())
							return;

						AddToHierarchy(InHierarchy, InChild, InID, false, int32_t(nodeIndex));
					});

					InID.entity().children([this, &InID, &InHierarchy, nodeIndex = InHierarchy.size() - 1](flecs::entity InChild)
					{
						AddToHierarchy(InHierarchy, InChild, InID, true, int32_t(nodeIndex));
					});
				}
			}
			else
			{
				auto& node = InHierarchy.emplace_back(SceneHierarchyNode{ id, false, InExpandable, InParentIndex });
				YUKI_UNUSED(node);
			}
		}

		void RebuildSceneHierarchy()
		{
			Yuki::DynamicArray<SceneHierarchyNode> newHierarchy;
			auto filter = m_World.GetEntityWorld().filter<Yuki::Components::Translation>();
			filter.each([&](flecs::entity InEntity, Yuki::Components::Translation& InTranslation)
			{
				if (InEntity.parent() != flecs::entity::null())
					return;

				AddToHierarchy(newHierarchy, InEntity, flecs::id{}, true);
			});

			m_SceneHierarchyCache = newHierarchy;
			m_SceneHierarchyCacheIndexLookup.clear();

			for (size_t i = 0; i < m_SceneHierarchyCache.size(); i++)
				m_SceneHierarchyCacheIndexLookup[m_SceneHierarchyCache[i].ID] = i;
		}

		size_t GetHierarchyDepth(int32_t InNodeIndex)
		{
			size_t depth = 0;
			while ((InNodeIndex = m_SceneHierarchyCache[InNodeIndex].ParentIndex) >= 0)
				depth++;
			return depth;
		}

		void DrawEntityListItem(SceneHierarchyNode& InNode, int32_t InNodeIndex, size_t& InLastDepth)
		{
			size_t depth = GetHierarchyDepth(InNodeIndex);
			for (; depth > InLastDepth; InLastDepth++)
				ImGui::Indent();
			for (; depth < InLastDepth; InLastDepth--)
				ImGui::Unindent();

			std::string label = "";

			auto id = InNode.ID.Child;

			const auto* name = id.entity().get<Yuki::Components::Name>();
			if (name)
				label = fmt::format("{}##{}", name->Value, id.raw_id());
			else
				label = fmt::format("{}##{}", id.entity().name(), id.raw_id());

			if (InNode.Expandable)
			{
				ImGui::ArrowButton("##ArrowButton", InNode.Expanded ? ImGuiDir_Down : ImGuiDir_Right);

				if (ImGui::IsItemClicked())
				{
					InNode.Expanded = !InNode.Expanded;
					m_RebuildSceneHierarchy = true;
				}
			}
			else
			{
				ImGui::Dummy(ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
			}

			ImGui::SameLine();
			ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap);

			if (ImGui::IsItemClicked())
				static_cast<EntityDetails*>(m_EditorPanels[2].get())->SetCurrentEntity(id.entity());
		}

		void DrawEntityList()
		{
			YUKI_SCOPE_EXIT_GUARD(){ ImGui::End(); };

			if (m_RebuildSceneHierarchy)
			{
				RebuildSceneHierarchy();
				m_RebuildSceneHierarchy = false;
			}

			if (!ImGui::Begin("Entities"))
				return;

			if (ImGui::Button("New Entity"))
			{
				/*auto entity = CreateEntity("New Entity");
				YUKI_UNUSED(entity);*/
			}

			ImGui::Separator();

			size_t lastDepth = 0;

			ImGuiListClipper clipper;
			clipper.Begin(int32_t(m_SceneHierarchyCache.size()));

			while (clipper.Step())
			{
				for (size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					auto& node = m_SceneHierarchyCache[i];
					DrawEntityListItem(node, int32_t(i), lastDepth);
				}
			}
		}

		void OnDestroy() override
		{
		}

	private:
		Yuki::Unique<Yuki::RenderContext> m_RenderContext = nullptr;

		std::vector<Yuki::GenericWindow*> m_Windows;
		Yuki::Queue m_GraphicsQueue{};
		Yuki::Fence m_Fence{};

		Yuki::Unique<Yuki::AssetSystem> m_AssetSystem = nullptr;
		Yuki::Unique<Yuki::AssetRegistry> m_AssetRegistry = nullptr;

		Yuki::Unique<Yuki::ImGuiWindowContext> m_ImGuiWindowContext;
		Yuki::Unique<Yuki::ImGuiRenderContext> m_ImGuiRenderContext;

		Yuki::CommandPool m_CommandPool{};
		Yuki::CommandList m_ImGuiCommandList{};

		Yuki::World m_World;

		Yuki::DynamicArray<SceneHierarchyNode> m_SceneHierarchyCache;
		Yuki::Map<HierarchyNodeID, size_t, HierarchyNodeIDHash> m_SceneHierarchyCacheIndexLookup;
		bool m_RebuildSceneHierarchy = false;

		Yuki::Map<flecs::entity, Yuki::Math::Vec3, Yuki::FlecsEntityHash> m_EulerCache;

		Yuki::DynamicArray<std::unique_ptr<EditorPanel>> m_EditorPanels;
	};
}


YUKI_DECLARE_APPLICATION(YukiEditor::TestApplication)
