#pragma once

namespace Yuki {

	template<typename T>
	struct Handle
	{
		using ID = uintptr_t;

		struct Impl;

		Handle() = default;
		Handle(Impl* impl)
			: m_Impl(impl), m_ID(reinterpret_cast<ID>(impl))
		{
		}

		T Unwrap() const noexcept { return T(m_Impl); }
		operator T() const noexcept { return Unwrap(); }

		operator bool() const noexcept { return m_Impl; }
		Impl* operator->() const noexcept { return m_Impl; }

		uintptr_t GetID() const { return m_ID; }

	protected:
		Impl* m_Impl = nullptr;
		ID m_ID = 0;
	};

}
