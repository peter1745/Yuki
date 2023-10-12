#pragma once

#include <functional>

namespace Yuki {

	class UniqueID
	{
	public:
		UniqueID();
		UniqueID(uint64_t value);

		operator uint64_t() { return m_Value; }
		operator const uint64_t() const { return m_Value; }

	private:
		uint64_t m_Value = 0;
	};

}

namespace std {

	template<>
	struct hash<Yuki::UniqueID>
	{
		size_t operator()(const Yuki::UniqueID& id) const
		{
			return static_cast<size_t>(id);
		}
	};

}
