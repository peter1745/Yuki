#include "RHI.hpp"

#include <comdef.h>

extern "C" {
	__declspec(dllexport) extern const UINT D3D12SDKVersion = 711;
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace Yuki {

	RHIContext RHIContext::Create()
	{
		auto* context = new Impl();

		ID3D12Debug* debugController;
		CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		CheckHR(debugController->QueryInterface(IID_PPV_ARGS(&context->DebugController)));

		context->DebugController->EnableDebugLayer();

		CheckHR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&context->Factory)));

		IDXGIAdapter1* dxgiAdapter;

		for (uint32_t i = 0; context->Factory->EnumAdapters1(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 desc;
			dxgiAdapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (FAILED(D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&context->Device))))
			{
				continue;
			}

			D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions5 = {};
			CheckHR(context->Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureOptions5, sizeof(featureOptions5)));

			if (featureOptions5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
			{
				continue;
			}

			D3D12_FEATURE_DATA_D3D12_OPTIONS12 featureOptions12 = {};
			CheckHR(context->Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &featureOptions12, sizeof(featureOptions12)));

			if (!featureOptions12.EnhancedBarriersSupported)
			{
				continue;
			}

			WriteLine("GPU: {}", Utf16ToUtf8(desc.Description));
			break;
		}

		return { context };
	}

}
