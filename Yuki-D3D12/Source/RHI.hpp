#pragma once

#include "Common.hpp"

#include <Engine/RHI/RHI.hpp>

namespace Yuki {

	template<>
	struct Handle<RHIContext>::Impl
	{
		ID3D12Debug1* DebugController;
		ID3D12DebugDevice1* DebugDevice;

		IDXGIFactory6* Factory;
		ID3D12Device* Device;
	};

}
