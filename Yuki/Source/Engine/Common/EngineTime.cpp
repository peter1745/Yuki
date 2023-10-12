#include "EngineTime.hpp"

namespace Yuki {

	void EngineTime::CalculateDeltaTime()
	{
		auto& self = GetInternal();
		auto now = Clock::now();
		self.m_DeltaTime = std::chrono::duration_cast<Duration>(now - self.m_LastTime).count();
		self.m_LastTime = now;
	}

}
