#pragma once

namespace Yuki {

	template<typename T>
	struct Handle
	{
		struct Impl;

		Handle() = default;
		Handle(Impl* impl)
			: m_Impl(impl) {}

		T Unwrap() const noexcept { return T(m_Impl); }
		operator T() const noexcept { return Unwrap(); }

		operator bool() const noexcept { return m_Impl; }
		Impl* operator->() const noexcept { return m_Impl; }

	protected:
		Impl* m_Impl = nullptr;
	};

}
