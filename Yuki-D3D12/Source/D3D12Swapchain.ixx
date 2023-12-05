module;

#include "D3D12Common.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module D3D12:Swapchain;

import Aura;
import Yuki.Core;
import Yuki.Rendering;
import Yuki.Windows;

import :RHIHandles;

namespace Yuki {

	Swapchain Swapchain::Create(RHIContext context, Queue queue, Window window)
	{
		auto* impl = new Impl();

		RECT clientRect = {};
		GetClientRect(window->NativeHandle, &clientRect);

		DXGI_SWAP_CHAIN_DESC swapchainDesc =
		{
			.BufferDesc = {
				.Width = static_cast<UINT>(clientRect.right),
				.Height = static_cast<UINT>(clientRect.bottom),
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			},
			.SampleDesc = {
				.Count = 1,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 3,
			.OutputWindow = window->NativeHandle,
			.Windowed = TRUE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
		};

		IDXGISwapChain* swapchain;
		CheckHR(context->Factory->CreateSwapChain(queue->Handle, &swapchainDesc, &swapchain));
		CheckHR(swapchain->QueryInterface(&impl->Handle));
		CheckHR(context->Factory->MakeWindowAssociation(window->NativeHandle, DXGI_MWA_NO_ALT_ENTER));

		return { impl };
	}

	void Swapchain::Present() const
	{
		CheckHR(m_Impl->Handle->Present(1, 0));
	}

}
