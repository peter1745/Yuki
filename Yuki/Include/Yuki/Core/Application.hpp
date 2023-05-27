#pragma once

#include "GenericWindow.hpp"

#include "Yuki/EventSystem/EventSystem.hpp"
#include "Yuki/Rendering/RHI/RenderContext.hpp"
#include "Yuki/Rendering/Renderer.hpp"

namespace Yuki {

	class Application
	{
	public:
		Application(const std::string& InName, RenderAPI InRenderAPI = RenderAPI::Vulkan);
		virtual ~Application() = default;

		int32_t GetExitCode() const { return m_ExitCode; }

		EventSystem& GetEventSystem() const { return *m_EventSystem; }

		GenericWindow* NewWindow(WindowAttributes InWindowAttributes);

		RenderContext* GetRenderContext() const { return m_RenderContext.GetPtr(); }

		RenderAPI GetRenderAPI() const { return m_RenderingAPI; }

	private:
		virtual void OnInitialize() {}
		virtual void OnRunLoop() {}
		virtual void OnDestroy() {}

	private:
		void Initialize();
		void Run();
		void Destroy();

		void OnWindowClose(const WindowCloseEvent& InEvent);

	private:
		std::string m_Name;
		RenderAPI m_RenderingAPI = RenderAPI::Vulkan;

		List<Unique<GenericWindow>> m_Windows;
		GenericWindow* m_MainWindow = nullptr;
		Unique<EventSystem> m_EventSystem = nullptr;
		Unique<RenderContext> m_RenderContext = nullptr;

		Unique<Renderer> m_Renderer = nullptr;

		CommandBuffer* m_CommandBuffer;

		Fence* m_Fence = nullptr;

		bool m_RunEngineLoop = false;

		int32_t m_ExitCode = 0;

		List<GenericWindow*> m_ClosedWindows;

	private:
		template<typename TAppClass>
		friend class EntryPoint;
	};

}
