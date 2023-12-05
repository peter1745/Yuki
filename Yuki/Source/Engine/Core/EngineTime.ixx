module;

#include <chrono>

export module Yuki.Core:EngineTime;

using namespace std::chrono;
using namespace std::chrono_literals;

export namespace Yuki {

	class EngineTime
	{
	public:
		float GetDeltaTime() const { return s_DeltaTime; }

	private:
		EngineTime()
		{
			m_LastTime = steady_clock::now();
		}

		void CalculateDeltaTime()
		{
			const auto now = steady_clock::now();
			s_DeltaTime = duration_cast<duration<float, std::milli>>(now - m_LastTime).count();
			m_LastTime = now;
		}

	private:
		inline static std::atomic<float> s_DeltaTime;
		steady_clock::time_point m_LastTime;

	private:
		friend class Application;
	};

}
