#pragma once

#include "Unique.hpp"

#include "Engine/Input/InputSystem.hpp"

#include <concepts>
#include <filesystem>

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
		virtual void OnShutdown() {}

	private:
		void Run();

	protected:
		bool m_Running = true;

		std::filesystem::path m_BaseDirectory;

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
		AppRunner(const std::filesystem::path& filepath)
			: m_Application(new T())
		{
			m_Application->m_BaseDirectory = filepath;
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
