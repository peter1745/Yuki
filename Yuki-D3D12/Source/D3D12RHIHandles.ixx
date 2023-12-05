module;

#include <vector>

#include "D3D12Common.hpp"

export module D3D12:RHIHandles;

import Yuki.Rendering;
import Aura;

export {

	template<>
	struct Aura::HandleImpl<Yuki::RHIContext>
	{
		ID3D12Device* Device;
		IDXGIFactory6* Factory;
	};

	template<>
	struct Aura::HandleImpl<Yuki::Shader>
	{
		IDxcBlob* Handle;
	};

	template<>
	struct Aura::HandleImpl<Yuki::ShaderLibrary>
	{
		IDxcUtils* DxcUtils;
		IDxcCompiler3* Compiler;
	};

	template<>
	struct Aura::HandleImpl<Yuki::Fence>
	{
		ID3D12Fence1* Handle;
		HANDLE EventHandle;
		uint64_t Value;
	};

	template<>
	struct Aura::HandleImpl<Yuki::Queue>
	{
		D3D12_COMMAND_LIST_TYPE Type;
		ID3D12CommandQueue* Handle;
	};

	template<>
	struct Aura::HandleImpl<Yuki::Swapchain>
	{
		IDXGISwapChain3* Handle;
	};

	template<>
	struct Aura::HandleImpl<Yuki::GraphicsPipeline>
	{
		ID3D12PipelineState* PipelineState;
	};

	template<>
	struct Aura::HandleImpl<Yuki::CommandList>
	{
		ID3D12GraphicsCommandList7* Handle;
	};

	template<>
	struct Aura::HandleImpl<Yuki::CommandAllocator>
	{
		Yuki::RHIContext Context;
		ID3D12CommandAllocator* Handle;

		std::vector<Yuki::CommandList> AllocatedLists;
		size_t NextListIdx;
	};

	namespace Yuki {

		inline constexpr D3D12_COMMAND_LIST_TYPE QueueTypeToD3D12CommandListType(QueueType queueType)
		{
			switch (queueType)
			{
			case QueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case QueueType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
			case QueueType::Transfer: return D3D12_COMMAND_LIST_TYPE_COPY;
			}

			__debugbreak();
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}

	}

}
