#pragma once

#include "Unique.hpp"

#include "Engine/Input/InputSystem.hpp"

#include <concepts>

namespace Yuki {

	class WindowSystem;

	class Application
	{
	public:
		Application();

		virtual ~Application() = default;

	protected:
		virtual void OnRun() {}
		virtual void OnUpdate() {}

	private:
		void Run();

	protected:
		bool m_Running = true;

		Unique<WindowSystem> m_WindowSystem = nullptr;
		InputSystem m_InputSystem;

	private:
		template<std::derived_from<Application> T>
		friend class AppRunner;
	};

	template<std::derived_from<Application> T>
	class AppRunner final
	{
	public:
		AppRunner()
			: m_Application(new T())
		{
		}

		~AppRunner()
		{
			delete m_Application;
		}

		void Run()
		{
			m_Application->Run();
		}

	private:
		T* m_Application = nullptr;
	};

}
