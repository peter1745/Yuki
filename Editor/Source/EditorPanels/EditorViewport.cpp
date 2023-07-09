#include "EditorViewport.hpp"
#include "../FreeCamera.hpp"

#include <Yuki/Core/ScopeExitGuard.hpp>
#include <Yuki/Rendering/EntityRenderer.hpp>
#include <Yuki/ImGui/ImGuiRenderContext.hpp>

#include <imgui/imgui.h>

namespace YukiEditor {

	EditorViewport::EditorViewport(Yuki::GenericWindow* InWindow, Yuki::RenderContext* InContext, Yuki::ImGuiRenderContext* InImGuiContext)
		: m_Context(InContext), m_ImGuiContext(InImGuiContext)
	{
		m_Renderer = Yuki::Unique<Yuki::WorldRenderer>::Create(m_Context);
		m_Camera = Yuki::Unique<FreeCamera>::Create(InWindow);
	}

	void EditorViewport::Update(float InDeltaTime)
	{
		m_Camera->Update(InDeltaTime);

		m_Renderer->Reset();

		if (m_ViewportWidth != m_LastViewportWidth || m_ViewportHeight != m_LastViewportHeight)
		{
			m_Renderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_ImGuiContext->RecreateImage(m_Renderer->GetFinalImage());
		}

		m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, (float)m_ViewportWidth / m_ViewportHeight, 0.05f) * m_Camera->GetViewMatrix());
		m_Renderer->RenderEntities();
		m_Renderer->EndFrame();
	}

	void EditorViewport::Draw()
	{
		YUKI_SCOPE_EXIT_GUARD()
		{
			ImGui::PopStyleVar();
			ImGui::End();
		};

		m_LastViewportWidth = m_ViewportWidth;
		m_LastViewportHeight = m_ViewportHeight;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (!ImGui::Begin("Viewport"))
			return;

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ImGuiContext->DrawImage(m_Renderer->GetFinalImage(), viewportSize);

		if (uint32_t(viewportSize.x) > 0 && uint32_t(viewportSize.y) > 0)
		{
			m_ViewportWidth = uint32_t(viewportSize.x);
			m_ViewportHeight = uint32_t(viewportSize.y);
		}
	}

}
