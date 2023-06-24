#pragma once

#include "GenericWindow.hpp"

#include "Yuki/EventSystem/EventSystem.hpp"
#include "Yuki/Rendering/RenderAPI.hpp"
#include "Yuki/Rendering/RenderContext.hpp"

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
		virtual void OnRunLoop(float InDeltaTime) {}
		virtual void OnDestroy() {}

	private:
		void Initialize();
		void Run();
		void Destroy();

		void OnWindowClose(const WindowCloseEvent& InEvent);

	private:
		std::string m_Name;
		RenderAPI m_RenderingAPI = RenderAPI::Vulkan;

		DynamicArray<Unique<GenericWindow>> m_Windows;
		Unique<EventSystem> m_EventSystem = nullptr;
		Unique<RenderContext> m_RenderContext = nullptr;

		bool m_RunEngineLoop = false;

		int32_t m_ExitCode = 0;

		DynamicArray<GenericWindow*> m_ClosedWindows;

		float m_LastFrameTime = 0.0f;
		float m_DeltaTime = 0.0f;

	private:
		template<typename TAppClass>
		friend class EntryPoint;
	};

}
