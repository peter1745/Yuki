#include "EngineTime.hpp"

namespace Yuki {

	void EngineTime::CalculateDeltaTime()
	{
		auto& This = GetInternal();
		auto Now = Clock::now();
		This.m_DeltaTime = std::chrono::duration_cast<Duration>(Now - This.m_LastTime).count();
		This.m_LastTime = Now;
	}

}
