module;

#include "D3D12Common.hpp"

#include <cstdint>

export module D3D12:Fence;

import Aura;
import Yuki.Rendering;
import Yuki.Windows;

import :Context;

namespace Yuki {

	Fence Fence::Create(RHIContext context)
	{
		auto* impl = new Impl();

		CheckHR(context->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&impl->Handle)));
		impl->EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (!impl->EventHandle)
		{
			CheckHR(HRESULT_FROM_WIN32(GetLastError()));
		}

		return { impl };
	}

	void Fence::Wait(uint64_t value) const
	{
		uint64_t waitValue = value ? value : m_Impl->Value;

		if (m_Impl->Handle->GetCompletedValue() < waitValue)
		{
			CheckHR(m_Impl->Handle->SetEventOnCompletion(waitValue, m_Impl->EventHandle));
			WaitForSingleObject(m_Impl->EventHandle, INFINITE);
		}
	}

}
