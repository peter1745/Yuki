#pragma once

#include "Logging.hpp"

#include <chrono>

namespace Yuki {

	class Timer
	{
	public:
		using Clock = std::chrono::steady_clock;

	public:
		Timer(std::string_view InName)
			: m_Name(InName)
		{
			m_StartTime = Clock::now();
		}

		float Elapsed() const { return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(Clock::now() - m_StartTime).count(); }
		float ElapsedSeconds() const { return Elapsed() * 0.001f; }

	protected:
		std::string_view m_Name;
		Clock::time_point m_StartTime;
	};

	class ScopedTimer : public Timer
	{
	public:
		ScopedTimer(std::string_view InName)
			: Timer(InName)
		{
		}

		~ScopedTimer()
		{
			LogInfo("[{}]: {}ms", m_Name, Elapsed());
		}
	};

}
