module;

#include "D3D12Common.hpp"

#include <vector>

export module D3D12:CommandList;

import Aura;
import Yuki.Rendering;
import Yuki.Windows;

import :RHIHandles;

export namespace Yuki {

	CommandAllocator CommandAllocator::Create(RHIContext context)
	{
		auto* impl = new Impl();
		impl->Context = context;
		CheckHR(context->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&impl->Handle)));
		return { impl };
	}

	CommandList CommandAllocator::NewList() const
	{
		// Return an existing list if it's unused (should be in a recording state already)
		if (m_Impl->NextListIdx < m_Impl->AllocatedLists.size())
		{
			return m_Impl->AllocatedLists[m_Impl->NextListIdx++];
		}

		// Otherwise create a new list and add it to our AllocatedLists vector
		auto* impl = new CommandList::Impl();
		CheckHR(m_Impl->Context->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Impl->Handle, nullptr, IID_PPV_ARGS(&impl->Handle)));

		CommandList list = { impl };
		m_Impl->AllocatedLists.push_back(list);
		m_Impl->NextListIdx++;
		return list;
	}

	void CommandAllocator::Reset() const
	{
		CheckHR(m_Impl->Handle->Reset());

		for (auto& list : m_Impl->AllocatedLists)
		{
			list->Handle->Reset(m_Impl->Handle, nullptr);
		}

		m_Impl->NextListIdx = 0;
	}

	void CommandList::End() const
	{
		m_Impl->Handle->Close();
	}

}
