module;

#include "D3D12Common.hpp"

#include <iostream>

export module D3D12:Context;

import Aura;
import Yuki.Rendering;
import Yuki.Windows;

import :RHIHandles;

export {
	extern "C" {
		__declspec(dllexport) extern const UINT D3D12SDKVersion = 711;
		__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
	}
}

namespace Yuki {

	RHIContext RHIContext::Create()
	{
		auto* impl = new Impl();

		ID3D12Debug* debugController;
		CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();

		CheckHR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&impl->Factory)));

		IDXGIAdapter1* dxgiAdapter;

		for (uint32_t i = 0; impl->Factory->EnumAdapters1(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 desc;
			dxgiAdapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (FAILED(D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&impl->Device))))
			{
				continue;
			}

			D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions5 = {};
			impl->Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureOptions5, sizeof(featureOptions5));

			if (featureOptions5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
			{
				continue;
			}

			D3D12_FEATURE_DATA_D3D12_OPTIONS12 featureOptions12 = {};
			impl->Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &featureOptions12, sizeof(featureOptions12));

			if (!featureOptions12.EnhancedBarriersSupported)
			{
				continue;
			}

			std::wcout << L"Selected GPU: " << desc.Description << L"\n";
			break;
		}

		return { impl };
	}

	Queue RHIContext::RequestQueue(QueueType queueType) const
	{
		auto* impl = new Queue::Impl();
		impl->Type = QueueTypeToD3D12CommandListType(queueType);

		D3D12_COMMAND_QUEUE_DESC commandQueueDesc =
		{
			.Type = impl->Type,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		};
		CheckHR(m_Impl->Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&impl->Handle)));

		return { impl };
	}
}
