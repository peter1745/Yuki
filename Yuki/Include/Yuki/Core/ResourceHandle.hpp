#pragma once

#include "Core.hpp"

namespace Yuki {

	template<typename TResource>
	class ResourceHandle;
	class ResourceHandleFactory;

	template <typename TResource>
	struct ResourceHandleHash
	{
		using is_avalanching = void;

		uint64_t operator()(const ResourceHandle<TResource>& InHandle) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InHandle, sizeof(InHandle));
		}
	};

	template<typename TResource>
	class ResourceHandle
	{
	public:
		using HashType = ResourceHandleHash<TResource>;
		static const ResourceHandle<TResource> Invalid;

	public:
		ResourceHandle()
		    : m_Handle(ResourceHandleFactory::NewHandle()) {}

		ResourceHandle(const ResourceHandle& InOther)
		    : m_Handle(InOther.m_Handle) {}

		ResourceHandle(ResourceHandle&& InOther) noexcept
			: m_Handle(InOther.m_Handle)
		{
			InOther.m_Handle = Invalid.m_Handle;
		}

		~ResourceHandle() = default;

		ResourceHandle& operator=(const ResourceHandle& InOther)
		{
			m_Handle = InOther.m_Handle;
			return *this;
		}

		ResourceHandle& operator=(ResourceHandle&& InOther) noexcept
		{
			m_Handle = InOther.m_Handle;
			InOther.m_Handle = Invalid.m_Handle;
			return *this;
		}

		bool operator==(const ResourceHandle& InOther) const { return m_Handle == InOther.m_Handle; }
		bool operator!=(const ResourceHandle& InOther) const { return m_Handle != InOther.m_Handle; }

		bool IsValid() const { return m_Handle != std::numeric_limits<uint32_t>::max(); }

	public:
		template<typename TOther>
		ResourceHandle(const ResourceHandle<TOther>&) = delete;
		template <typename TOther>
		ResourceHandle& operator=(const ResourceHandle<TOther>&) = delete;

		template <typename TOther>
		ResourceHandle(ResourceHandle<TOther>&&) = delete;
		template <typename TOther>
		ResourceHandle& operator=(ResourceHandle<TOther>&&) = delete;

	private:
		ResourceHandle(uint32_t InHandle)
		    : m_Handle(InHandle) {}

	private:
		uint32_t m_Handle = 0;
	};

	template <typename TResource>
	__declspec(selectany) const ResourceHandle<TResource> ResourceHandle<TResource>::Invalid = ResourceHandle<TResource>(std::numeric_limits<uint32_t>::max());

	class ResourceHandleFactory
	{
	private:
		static uint32_t NewHandle();

		template<typename TResource>
		friend class ResourceHandle;
	};

}

