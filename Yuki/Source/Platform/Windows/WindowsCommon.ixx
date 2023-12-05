module;

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <comdef.h>

export module Yuki.Windows;

export import :Windowing;

export namespace Yuki {

	inline void CheckHR(HRESULT result)
	{
		if (FAILED(result))
		{
			_com_error err(result);
			OutputDebugString(err.ErrorMessage());
			__debugbreak();
		}
	}

}
