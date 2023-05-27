#pragma once

namespace Yuki {

	class GenericWindow;
	class Swapchain;
	class CommandBuffer;

	class Viewport
	{
	public:
		virtual ~Viewport() = default;

		virtual GenericWindow* GetWindow() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual Swapchain* GetSwapchain() const = 0;

		virtual void AcquireNextImage() = 0;
		virtual void RecreateSwapchain() = 0;

	};

}
