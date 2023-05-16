#pragma once

#include "GenericWindow.hpp"

namespace Yuki {

	class Application
	{
	public:
		Application(const std::string& InName);
		virtual ~Application() = default;

		int32_t GetExitCode() const { return m_ExitCode; }

	private:
		virtual void OnInitialize() {}
		virtual void OnRunLoop() {}

	private:
		void Initialize();
		void Run();

	private:
		std::string m_Name;
		Unique<GenericWindow> m_Window = nullptr;
		int32_t m_ExitCode = 0;

	private:
		template<typename TAppClass>
		friend class EntryPoint;
	};

}
