#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"
#include "Yuki/Rendering/ImageFormat.hpp"
#include "Yuki/Rendering/RHI/CommandBufferPool.hpp"
#include "Yuki/Rendering/RHI/Buffer.hpp"
#include "Yuki/Rendering/RHI/DescriptorSet.hpp"

#include "Yuki/Memory/Unique.hpp"

#include <span>

namespace Yuki {

	class GenericWindow;
	class Viewport;
	class ShaderManager;
	class ShaderCompiler;
	class GraphicsPipelineBuilder;
	class SetLayoutBuilder;
	class Image2D;
	class ImageView2D;
	class RenderInterface;
	class Queue;
	class Fence;
	class Sampler;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual ShaderManager* GetShaderManager() const = 0;
		virtual ShaderCompiler* GetShaderCompiler() const = 0;

		virtual Queue* GetGraphicsQueue() const = 0;

		virtual void WaitDeviceIdle() const = 0;

	public:
		virtual void ResetCommandPool() = 0;

		virtual RenderInterface* CreateRenderInterface() = 0;
		virtual void DestroyRenderInterface(RenderInterface* InRenderInterface) = 0;

		virtual GraphicsPipelineBuilder* CreateGraphicsPipelineBuilder() = 0;
		virtual void DestroyGraphicsPipelineBuilder(GraphicsPipelineBuilder* InPipelineBuilder) = 0;

		virtual SetLayoutBuilder* CreateSetLayoutBuilder() = 0;
		virtual void DestroySetLayoutBuilder(SetLayoutBuilder* InSetLayoutBuilder) = 0;

		virtual DescriptorPool* CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) = 0;
		virtual void DestroyDescriptorPool(DescriptorPool* InDescriptorPool) = 0;

		virtual Viewport* CreateViewport(GenericWindow* InWindow) = 0;
		virtual void DestroyViewport(Viewport* InViewport) = 0;

		virtual Image2D* CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat) = 0;
		virtual void DestroyImage2D(Image2D* InImage) = 0;
		
		virtual ImageView2D* CreateImageView2D(Image2D* InImage) = 0;
		virtual void DestroyImageView2D(ImageView2D* InImageView) = 0;

		virtual Sampler* CreateSampler() = 0;
		virtual void DestroySampler(Sampler* InSampler) = 0;

		virtual Buffer* CreateBuffer(const BufferInfo& InInfo) = 0;
		virtual void DestroyBuffer(Buffer* InBuffer) = 0;
		
		virtual Fence* CreateFence() = 0;
		virtual void DestroyFence(Fence* InFence) = 0;
		
		virtual CommandBufferPool* CreateCommandBufferPool(CommandBufferPoolInfo InInfo) = 0;
		virtual void DestroyCommandBufferPool(CommandBufferPool* InCommandBufferPool) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InRenderAPI);
	};

}
