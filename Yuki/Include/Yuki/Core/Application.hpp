#pragma once

#include "GenericWindow.hpp"

#include "../EventSystem/EventSystem.hpp"
#include "../Rendering/RenderContext.hpp"

namespace Yuki {

	class Application
	{
	public:
		Application(const std::string& InName);
		virtual ~Application() = default;

		int32_t GetExitCode() const { return m_ExitCode; }

		EventSystem& GetEventSystem() const { return *m_EventSystem; }

	private:
		virtual void OnInitialize() {}
		virtual void OnRunLoop() {}

	private:
		void Initialize();
		void Run();
		void Destroy();

		void OnWindowClose(const WindowCloseEvent& InEvent);

	private:
		std::string m_Name;
		Unique<GenericWindow> m_Window = nullptr;
		Unique<EventSystem> m_EventSystem = nullptr;
		Unique<RenderContext> m_RenderContext = nullptr;

		bool m_RunEngineLoop = false;

		int32_t m_ExitCode = 0;

	private:
		template<typename TAppClass>
		friend class EntryPoint;
	};

}
