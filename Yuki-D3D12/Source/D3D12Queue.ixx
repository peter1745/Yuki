export module D3D12:Queue;

import Aura;
import Yuki.Rendering;
import Yuki.Windows;

import :RHIHandles;

/*#define AURA_USE_LARGE_STACK
#include <Aura/Stack.hpp>*/

namespace Yuki {
	
	void Queue::ExecuteCommandLists(const Aura::Span<CommandList> commandLists, const Aura::Span<Fence> signalFences) const
	{
		/*AuraStackPoint();

		auto d3dLists = AuraStackAlloc(ID3D12CommandList*, commandLists.Count());
		
		for (size_t i = 0; i < commandLists.Count(); i++)
		{
			commandLists[i].End();

			d3dLists[i] = commandLists[i]->Handle;
		}

		m_Impl->Handle->ExecuteCommandLists(static_cast<UINT>(d3dLists.Count()), d3dLists.Begin());

		for (auto& fence : signalFences)
		{
			CheckHR(m_Impl->Handle->Signal(fence->Handle, ++fence->Value));
		}*/
	}

}
