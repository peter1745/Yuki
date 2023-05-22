#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"
#include "Yuki/Rendering/ImageFormat.hpp"

#include "Yuki/Memory/Unique.hpp"

namespace Yuki {

	class GenericWindow;
	class Swapchain;
	class ShaderManager;
	class ShaderCompiler;
	class Image2D;
	class ImageView2D;
	class RenderTarget;
	struct RenderTargetInfo;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual ShaderManager* GetShaderManager() const = 0;
		virtual ShaderCompiler* GetShaderCompiler() const = 0;

	public:
		virtual Swapchain* CreateSwapchain(GenericWindow* InWindow) = 0;
		virtual Image2D* CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat) = 0;
		virtual ImageView2D* CreateImageView2D(Image2D* InImage) = 0;
		virtual RenderTarget* CreateRenderTarget(const RenderTargetInfo& InInfo) = 0;

		virtual void DestroySwapchain(Swapchain* InSwapchain) = 0;
		virtual void DestroyImage2D(Image2D* InImage) = 0;
		virtual void DestroyImageView2D(ImageView2D* InImageView) = 0;
		virtual void DestroyRenderTarget(RenderTarget* InRenderTarget) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InRenderAPI);
	};

}
