#pragma once

#include "Core.hpp"

#include <chrono>

namespace Yuki {

	class EngineTime
	{
		YUKI_SINGLETON(EngineTime);

		using Clock = std::chrono::steady_clock;
		using Duration = std::chrono::duration<double, std::milli>;

	public:
		template<std::floating_point T = double>
		static T DeltaTime() { return static_cast<T>(Get().m_DeltaTime / 1000.0); }

	private:
		static void CalculateDeltaTime();

	private:
		Duration::rep m_DeltaTime = {};
		Clock::time_point m_LastTime = {};

	private:
		friend class Application;
	};

}
