#pragma once

namespace Yuki {

	struct MessageBase {};

	struct MessageTraits
	{
	private:
		inline static size_t IndexInternal()
		{
			static size_t s_Index = 0;
			return s_Index++;
		}

	public:
		template<typename TMessageClass>
		inline static size_t Index()
		{
			static size_t s_Index = IndexInternal();
			return s_Index;
		}
	};

}
