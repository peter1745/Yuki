#pragma once

namespace Yuki {

	struct CommandBuffer
	{
		void* Instance;

		template<typename T>
		T As() const { return reinterpret_cast<T>(Instance); }
	};

}
