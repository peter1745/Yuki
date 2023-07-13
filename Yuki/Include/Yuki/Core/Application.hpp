#pragma once

#include "GenericWindow.hpp"

#include "Yuki/EventSystem/EventSystem.hpp"

namespace Yuki {

	class RenderContext;

	class Application
	{
	public:
		Application(const std::string& InName);
		virtual ~Application() = default;

		int32_t GetExitCode() const { return m_ExitCode; }

		EventSystem& GetEventSystem() const { return *m_EventSystem; }

		GenericWindow* NewWindow(RenderContext* InContext, WindowAttributes InWindowAttributes);

		float GetLastFrameTime() const { return m_LastFrameTime; }
		float GetDeltaTime() const { return m_DeltaTime; }

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

		DynamicArray<Unique<GenericWindow>> m_Windows;
		Unique<EventSystem> m_EventSystem = nullptr;

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
