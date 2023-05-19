#pragma once

#include <ankerl/unordered_dense.h>

namespace Yuki {

	template<typename TResource>
	class ResourceHandle;

	template <typename TResource>
	struct ResourceHandleHash
	{
		using is_avalanching = void;

		uint64_t operator()(const ResourceHandle<TResource>& InHandle) const noexcept
		{
			static_assert(std::has_unique_object_representations_v<ResourceHandle<TResource>>);
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
		ResourceHandle() = delete;

		ResourceHandle(const ResourceHandle& InOther)
		    : m_Handle(InOther.m_Handle) {}

		~ResourceHandle() = default;

		ResourceHandle& operator=(const ResourceHandle& InOther)
		{
			m_Handle = InOther.m_Handle;
			return *this;
		}

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

