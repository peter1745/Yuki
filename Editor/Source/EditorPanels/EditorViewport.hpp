#pragma once

#include "EditorPanel.hpp"

#include <Yuki/Memory/Unique.hpp>
#include <Yuki/Asset/AssetSystem.hpp>

namespace Yuki {
	class RenderContext;
	class ImGuiRenderContext;
	class World;
	class WorldRenderer;
	class GenericWindow;
}

namespace YukiEditor {

	class FreeCamera;

	class EditorViewport : public EditorPanel
	{
	public:
		EditorViewport(Yuki::AssetSystem& InAssetSystem, Yuki::GenericWindow* InWindow, Yuki::RenderContext* InContext, Yuki::ImGuiRenderContext* InImGuiContext, Yuki::World* InWorld);
		
		void SetWorld(Yuki::World* InWorld) { m_World = InWorld; }

		void Update(float InDeltaTime) override;
		void Draw() override;

		Yuki::WorldRenderer* GetRenderer() const { return m_Renderer.GetPtr(); }

	private:
		Yuki::RenderContext* m_Context = nullptr;
		Yuki::ImGuiRenderContext* m_ImGuiContext = nullptr;
		Yuki::Unique<Yuki::WorldRenderer> m_Renderer = nullptr;

		Yuki::AssetSystem& m_AssetSystem;

		uint32_t m_ViewportWidth, m_ViewportHeight;
		uint32_t m_LastViewportWidth, m_LastViewportHeight;

		Yuki::Unique<FreeCamera> m_Camera = nullptr;

		Yuki::World* m_World = nullptr;
	};

}
