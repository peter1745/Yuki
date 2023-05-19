#include "Core/ResourceHandle.hpp"

namespace Yuki {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution;

	uint32_t ResourceHandleFactory::NewHandle()
	{
		return s_UniformDistribution(s_RandomEngine);
	}

}
