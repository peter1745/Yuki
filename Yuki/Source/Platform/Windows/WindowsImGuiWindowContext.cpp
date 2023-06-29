#include "ImGui/ImGuiWindowContext.hpp"
#include "WindowsWindow.hpp"

#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Yuki {

	ImGuiWindowContext::ImGuiWindowContext(GenericWindow* InWindow)
	{
		auto* window = static_cast<WindowsWindow*>(InWindow);
		window->SetWndProcCallback(ImGui_ImplWin32_WndProcHandler);
		ImGui_ImplWin32_Init(window->GetWindowHandle());
	}

	ImGuiWindowContext::~ImGuiWindowContext()
	{

	}

	void ImGuiWindowContext::NewFrame() const
	{
		ImGui_ImplWin32_NewFrame();
	}

}
