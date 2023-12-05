module;

#include <concepts>

export module Yuki.Core:App;

import :EngineTime;

export namespace Yuki {

	class Application;

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

	class Application
	{
	public:
		virtual ~Application() = default;

	protected:
		virtual void OnRun() {}
		virtual void OnUpdate() {}

	private:
		void Run()
		{
			OnRun();

			while (m_Running)
			{
				m_EngineTime.CalculateDeltaTime();

				OnUpdate();
			}
		}

	protected:
		bool m_Running = true;

	private:
		EngineTime m_EngineTime;

	private:
		template<std::derived_from<Application> T>
		friend class AppRunner;
	};

}
